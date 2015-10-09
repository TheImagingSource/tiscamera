
#ifndef CPROPERTIESDIALOG_H
#define CPROPERTIESDIALOG_H

#include <QtGui/QMainWindow>
#include <QtGui/QLabel>
#include <QtGui/QAction>
#include <QtGui>

#include <QDialog>
#include "camerabase.h"
#include <vector>

class CPropertyControl
{
  public:
    QLabel *Label;
    QSlider *Slider;
    QCheckBox *Check;
    QPushButton *Push;
    int Property;
    
    CPropertyControl()
    {
      Label = NULL;
      Slider = NULL;
      Check = NULL;
      Push = NULL;
      Property = 0;
    }
    
    ~CPropertyControl()
    {
      if( Label != NULL )
	delete Label;
      if( Slider != NULL )
	delete Slider;
      if( Check != NULL )
	delete Check;
      if( Push != NULL )
	delete Push;
    }
    
};

  
class CPropertiesDialog : public QDialog
{
  Q_OBJECT
  
  public:
      CPropertiesDialog( QWidget *parent);
      ~CPropertiesDialog();
      void SetCamera(CCamera *pCamera);

  private slots:
    void OnMovedSlider(int value);
    void OnClickedCheckkBox(int); 
    void OnPushButton();
    

      
  private:
    QVBoxLayout *_PropertiesLayout;
    QPushButton *closeButton;
    CCamera *_pCamera;
    std::vector<CPropertyControl*> _PropertySliders;
    void deleteControls();
    
};

#endif // CPROPERTIESDIALOG_H
