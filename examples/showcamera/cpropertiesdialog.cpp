/*
 * Copyright 2015 bvtest <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "cpropertiesdialog.h"

CPropertiesDialog::CPropertiesDialog(QWidget *parent) : QDialog(parent)
{
  closeButton = new QPushButton(tr("Close"));
  _PropertiesLayout  = new QVBoxLayout();
  setLayout( _PropertiesLayout );
 
}


CPropertiesDialog::~CPropertiesDialog()
{
  g_print("Dialog Destructor calleectord\n");
  deleteControls();
}

void CPropertiesDialog::deleteControls()
{
  // Clean up controls
  for( std::vector<CPropertyControl*>::iterator it = _PropertySliders.begin(); it != _PropertySliders.end(); it++ )
  {
    delete *it;
  }
  _PropertySliders.clear();
}

void CPropertiesDialog::SetCamera(CCamera *pCamera )
{

  _pCamera = pCamera;
  
  setWindowTitle( _pCamera->_name);
  
  deleteControls();
  
  for( int i = 0; i < _pCamera->_cProperties.size(); i++ )
  {
    CPropertyControl *pSld = new CPropertyControl;
    switch( _pCamera->_cProperties[i]->_type )
    {
      case CProperty::BOOLEAN:
	pSld->Check = new QCheckBox(_pCamera->_cProperties[i]->_name );
	pSld->Check->setCheckState( _pCamera->_cProperties[i]->_value ? Qt::Checked : Qt::Unchecked );
	_PropertiesLayout->addWidget( pSld->Check);
	QObject::connect(pSld->Check, SIGNAL(stateChanged(int)), this, SLOT(OnClickedCheckkBox(int)));
	break;

      case CProperty::BUTTON:
	pSld->Push = new QPushButton(_pCamera->_cProperties[i]->_name );
	_PropertiesLayout->addWidget( pSld->Push);
	QObject::connect(pSld->Push, SIGNAL(released()), this, SLOT(OnPushButton()));
	break;

	
      default:
	pSld->Label = new QLabel( _pCamera->_cProperties[i]->_name);
	pSld->Slider = new QSlider(Qt::Horizontal);
	
	_PropertiesLayout->addWidget( pSld->Label);
	_PropertiesLayout->addWidget( pSld->Slider);
	pSld->Slider->setMinimum(_pCamera->_cProperties[i]->_minimum);
	pSld->Slider->setMaximum(_pCamera->_cProperties[i]->_maximum);
	pSld->Slider->setValue(_pCamera->_cProperties[i]->_value);
	QObject::connect(pSld->Slider, SIGNAL(sliderMoved(int)), this, SLOT(OnMovedSlider(int)));
	QObject::connect(pSld->Slider, SIGNAL(valueChanged(int)), this, SLOT(OnMovedSlider(int)));
	break;
    }
    
    pSld->Property = _pCamera->_cProperties[i]->_id;

    _PropertySliders.push_back( pSld );
  }
}



void CPropertiesDialog::OnMovedSlider(int value)
{
  int PropertyControl = -1;
  
  QSlider *pSender = (QSlider *)sender();
  for( int  i = 0; i < _PropertySliders.size(); i++ )
  {
    if( _PropertySliders[i]->Slider == pSender )
    {
      PropertyControl = i;
      break;
    }
  }
    
  if(PropertyControl >= 0 )
  {
    g_print("Slider %d Change %d\n",PropertyControl, value);
    _pCamera->SetProperty( _PropertySliders[PropertyControl]->Property, value);
  }

}

void CPropertiesDialog::OnClickedCheckkBox(int value)
{
  int PropertyControl = -1;
  
  QCheckBox *pSender = (QCheckBox*)sender();
  for( int  i = 0; i < _PropertySliders.size(); i++ )
  {
    if( _PropertySliders[i]->Check == pSender )
    {
      PropertyControl = i;
      break;
    }
  }
    
  if(PropertyControl >= 0 )
  {
      g_print("Checkbox %d Change %d\n",PropertyControl, value);
    
      if( _PropertySliders[PropertyControl]->Check->checkState() == Qt::Checked)
      {
	_pCamera->SetProperty( _PropertySliders[PropertyControl]->Property, 1);
      }
      else
      {
	_pCamera->SetProperty( _PropertySliders[PropertyControl]->Property, 0);
      }
  }
}


void CPropertiesDialog::OnPushButton()
{
  int PropertyControl = -1;
  
  QPushButton *pSender = (QPushButton*)sender();
  for( int  i = 0; i < _PropertySliders.size(); i++ )
  {
    if( _PropertySliders[i]->Push == pSender )
    {
      PropertyControl = i;
      break;
    }
  }
    
  if(PropertyControl >= 0 )
  {
     g_print("Push %d \n",PropertyControl);
    _pCamera->SetProperty( _PropertySliders[PropertyControl]->Property, 1);
  }
}

#include "cpropertiesdialog.moc"
