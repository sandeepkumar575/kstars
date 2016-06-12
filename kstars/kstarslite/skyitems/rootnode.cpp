#include <QSGTexture>
#include <QQuickWindow>

#include "rootnode.h"
#include "skymaplite.h"
#include "projections/projector.h"
#include "skymapcomposite.h"
#include "solarsystemcomposite.h"
#include "Options.h"

#include "constellationboundarylines.h"
#include "constellationlines.h"

#include "horizontalcoordinategrid.h"
#include "equatorialcoordinategrid.h"

#include "ecliptic.h"
#include "equator.h"

//SkyItems
#include "kstarslite/skyitems/planetsitem.h"
#include "kstarslite/skyitems/asteroidsitem.h"
#include "kstarslite/skyitems/cometsitem.h"
#include "kstarslite/skyitems/horizonitem.h"
#include "kstarslite/skyitems/lines/linesitem.h"
#include "kstarslite/skyitems/labelsitem.h"
#include "kstarslite/skyitems/constellationnamesitem.h"

//Lines
#include "kstarslite/skyitems/lines/equatoritem.h"
#include "kstarslite/skyitems/lines/eclipticitem.h"

RootNode::RootNode()
    :m_skyMapLite(SkyMapLite::Instance()),
      m_clipGeometry(0)
{
    genCachedTextures();
    Options::setProjection(Projector::Lambert);

    m_skyComposite = KStarsData::Instance()->skyComposite();
    m_solarSystem = m_skyComposite->solarSystemComposite();

    // LabelsItem needs to be created first so that other items could insert their labels in labelsList
    m_labelsItem = new LabelsItem(this);

    m_linesItem = new LinesItem(this);

    m_linesItem->addLinesComponent( m_skyComposite->equatorialCoordGrid(), "EquatorialGridColor", 1, Qt::DotLine );
    m_linesItem->addLinesComponent( m_skyComposite->horizontalCoordGrid(), "HorizontalGridColor", 2, Qt::DotLine );

    //m_linesItem->addLinesComponent( m_skyComposite->equator(), "EqColor", 1, Qt::SolidLine );
    m_equator = new EquatorItem(m_skyComposite->equator(),this);
    m_ecliptic = new EclipticItem(m_skyComposite->ecliptic(),this);
    m_linesItem->addLinesComponent( m_skyComposite->ecliptic(), "EclColor", 1, Qt::SolidLine );

    m_linesItem->addLinesComponent( m_skyComposite->constellationBoundary(), "CBoundColor", 1, Qt::SolidLine );
    m_linesItem->addLinesComponent( m_skyComposite->constellationLines(), "CLineColor", 1, Qt::SolidLine );

    m_planetsItem = new PlanetsItem(m_solarSystem->planets(), m_solarSystem->planetMoonsComponent(), this);
    m_asteroidsItem = new AsteroidsItem(m_solarSystem->asteroids(), this);
    m_cometsItem = new CometsItem(m_solarSystem->comets(), this);

    m_constelNamesItem = new ConstellationNamesItem(m_skyComposite->constellationNamesComponent(), this);

    m_horizonItem = new HorizonItem(m_skyComposite->horizon(), this);

    setIsRectangular(false);
    updateClipPoly();

    appendChildNode(m_labelsItem);

    /*
          m_linesItem(new LinesItem(this)), m_horizonItem(new HorizonItem(this))
     */
}

void RootNode::genCachedTextures() {
    QVector<QVector<QPixmap*>> images = m_skyMapLite->getImageCache();

    QQuickWindow *win = m_skyMapLite->window();

    m_textureCache = QVector<QVector<QSGTexture*>> (images.length());

    for(int i = 0; i < m_textureCache.length(); ++i) {
        int length = images[i].length();
        m_textureCache[i] = QVector<QSGTexture *>(length);
        for(int c = 1; c < length; ++c) {
            m_textureCache[i][c] = win->createTextureFromImage(images[i][c]->toImage(), QQuickWindow::TextureCanUseAtlas);
        }
    }
}

QSGTexture* RootNode::getCachedTexture(int size, char spType) {
    return m_textureCache[SkyMapLite::Instance()->harvardToIndex(spType)][size];
}

void RootNode::updateClipPoly() {
    QPolygonF newClip = m_skyMapLite->projector()->clipPoly();
    if(m_clipPoly != newClip) {
        m_clipPoly = newClip;
    } else {
        return; //We don't need to triangulate polygon and update geometry if polygon wasn't changed
    }
    QVector<QPointF> triangles;

    for(int i = 1; i < m_clipPoly.size() - 1; ++i) {
        triangles.append(m_clipPoly[0]);
        triangles.append(m_clipPoly[i]);
        triangles.append(m_clipPoly[i+1]);
    }

    const int size = triangles.size();
    if(!m_clipGeometry) {
        m_clipGeometry = new QSGGeometry (QSGGeometry::defaultAttributes_Point2D (),
                                          size);
        m_clipGeometry->setDrawingMode(GL_TRIANGLES);
        setGeometry(m_clipGeometry);
    } else {
        m_clipGeometry->allocate(size);
    }

    QSGGeometry::Point2D * vertex = m_clipGeometry->vertexDataAsPoint2D ();
    for (int i = 0; i < size; i++) {
        vertex[i].x = triangles[i].x();
        vertex[i].y = triangles[i].y();
    }
    markDirty(QSGNode::DirtyGeometry);
}

void RootNode::update() {
    updateClipPoly();
    //TODO: Move this check somewhere else (create a separate function)
    if(Options::showSolarSystem()) {
        m_planetsItem->update();
        if (!Options::showAsteroids() ) {
            if (m_asteroidsItem) delete m_asteroidsItem;
        } else {
            if(!m_asteroidsItem) m_asteroidsItem = new AsteroidsItem(m_solarSystem->asteroids(), this);
            m_asteroidsItem->update();
        }

        if (!Options::showComets() ) {
            if (m_cometsItem) delete m_cometsItem;
        } else {
            if(!m_cometsItem) m_cometsItem = new CometsItem(m_solarSystem->comets(), this);
            m_cometsItem->update();
        }
    } else {
        m_planetsItem->hide();
        if(m_asteroidsItem) m_asteroidsItem->hide();
        if(m_cometsItem) m_cometsItem->hide();
    }

    m_constelNamesItem->update();

    m_horizonItem->update();

    m_equator->update();
    m_ecliptic->update();

    m_linesItem->update();
    m_labelsItem->update();
}

