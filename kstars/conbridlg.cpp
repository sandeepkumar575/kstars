/***************************************************************************
                          conbridlg.h  -  Contrast/Brightness Dialog
                             -------------------
    begin                : Fri Feb 6th 2004
    copyright            : (C) 2004 by Jasem Mutlaq
    email                : mutlaqja@ikarustech.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  									   *
 ***************************************************************************/
 
 #include <klocale.h>
 #include <kimageeffect.h> 
 #include <kdebug.h>
 
 #include <qslider.h>
 #include <knuminput.h>
 
 #include <stdlib.h>
 
 #include "contrastbrightnessgui.h"
 #include "conbridlg.h"
 #include "fitsviewer.h"
 #include "fitsimage.h"
 
//TODO find a better and faster way to implement this, this operation can be memory and CPU intensive.

ContrastBrightnessDlg::ContrastBrightnessDlg(QWidget *parent) :
    KDialogBase(KDialogBase::Plain, i18n( "Brightness/Contrast" ), Ok|Cancel, Ok, parent )
{
    
  contrast = brightness = 0;
  viewer = (FITSViewer *) parent;
  width  = viewer->stats.width;
  height = viewer->stats.height;
  
  ConBriDlg = new ConBriForm(this);
  if (!ConBriDlg) return;
  
  localImgBuffer = (unsigned char *) malloc (width * height * sizeof(unsigned char));
  if (!localImgBuffer)
  {
    kdDebug() << "Not enough memory for local image buffer" << endl;
    return;
  }
  memcpy(localImgBuffer, viewer->image->reducedImgBuffer, width * height * sizeof(unsigned char));
  
  setMainWidget(ConBriDlg);
  this->show();
  
  connect(ConBriDlg->conSlider, SIGNAL( valueChanged(int)), this, SLOT (setContrast(int )));
  connect(ConBriDlg->briSlider, SIGNAL( valueChanged(int)), this, SLOT (setBrightness(int)));
  
}

ContrastBrightnessDlg::~ContrastBrightnessDlg()
{


}

void ContrastBrightnessDlg::setContrast(int contrastValue)
{
  int val=0;
  if (!viewer) return;

   // #1 Apply Contrast
   for (int i=0; i < height; i++)
           for (int j=0; j < width; j++)
	   {
		val  = localImgBuffer[ i * width + j];
		val = (val - 255.) * (-0.01 * -contrastValue) + val;
		if (val > 255) val = 255;
		if (val < 0) val = 0;
		viewer->image->currentImage->setPixel(j, i, qRgb(val, val, val));
	   }
	     
  // #2 Apply brightness
  if (brightness <= 64 && brightness >= -64)
    KImageEffect::intensity(*viewer->image->currentImage, brightness/64.);
  else if (brightness > 64)
  {
    KImageEffect::intensity(*viewer->image->currentImage, 1.);
    KImageEffect::intensity(*viewer->image->currentImage, (brightness - 64.) / 64.);
  }
  else if (brightness < -64)
  {
    KImageEffect::intensity(*viewer->image->currentImage, -1.);
    KImageEffect::intensity(*viewer->image->currentImage, (brightness + 64.) / 64.);
  }
  
  contrast = contrastValue;
  viewer->image->convertImageToPixmap();
  viewer->image->update();
  //viewer->paintEvent(NULL);
				  
}

void ContrastBrightnessDlg::setBrightness(int brightnessValue)
{
  int val = 0;
  if (!viewer) return;
  
  // #1 Apply Contrast
  for (int i=0; i < height; i++)
           for (int j=0; j < width; j++)
	   {
		val  = localImgBuffer[ i * width + j];
		val = (val - 255.) * (-0.01 * -contrast) + val;
		//val += brightness;
		if (val > 255) val = 255;
		if (val < 0) val = 0;
		
		viewer->image->currentImage->setPixel(j, i, qRgb(val, val, val));
	   }
  
  // #2 Apply brightness
  if (brightness <= 64 && brightness >= -64)
    KImageEffect::intensity(*viewer->image->currentImage, brightness/64.);
  else if (brightness > 64)
  {
    KImageEffect::intensity(*viewer->image->currentImage, 1.);
    KImageEffect::intensity(*viewer->image->currentImage, (brightness - 64.) / 64.);
  }
  else if (brightness < -64)
  {
    KImageEffect::intensity(*viewer->image->currentImage, -1.);
    KImageEffect::intensity(*viewer->image->currentImage, (brightness + 64.) / 64.);
  }
  
  brightness = brightnessValue;
  viewer->image->convertImageToPixmap();
  viewer->image->update();
  //viewer->paintEvent(NULL);

}

QSize ContrastBrightnessDlg::sizeHint() const
{
  return QSize(400,130);
}

