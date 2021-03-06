/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
/* -*- mode: c++ -*- */

/** @file

InspectorBase class implemenation for Qt Designer

Copyright (C) 2002-2004   The Board of Trustees of The Leland
Stanford Junior University.  All Rights Reserved.

$Id: InspectorBase.ui.h,v 1.297.2.1 2004/02/02 01:37:15 pfkeb Exp $

*/

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#ifdef _MSC_VER
#include "msdevstudio/MSconfig.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "QtView.h"

#include "controllers/CutController.h"
#include "controllers/DataRepController.h"
#include "controllers/FunctionController.h"
#include "transforms/PeriodicBinaryTransform.h"
#include "datasrcs/NTuple.h"
#include "functions/FunctionFactory.h"
#include "plotters/CutPlotter.h"
#include "plotters/TextPlotter.h"
#include "projectors/ProjectorBase.h"
#include "reps/ContourPointRep.h"
#include "reps/RepBase.h"

#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cmath>
#include <iostream>

using namespace hippodraw;

using std::cout;
using std::endl;
using std::list;
using std::min;
using std::string;
using std::vector;


void InspectorBase::init()
{
  QSize cur_size = size();
  setFixedSize ( cur_size );

  m_dragging = false;
  m_axis = Axes::X;
  m_layoutWidget = new QWidget( currentPlot, "m_Layout" );
  m_layoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_vLayout = new QVBoxLayout( m_layoutWidget, 0, 6, "m_vLayout");  

  newPlotButton->setEnabled( false );

  m_newLayoutWidget = new QWidget ( m_new_plot_box, "m_newLayout" );
  m_newLayoutWidget->setGeometry( QRect ( 7, 75, 360, 0 ) );
  m_newVLayout = new QVBoxLayout( m_newLayoutWidget, 0, 6,
				  "m_newVLayout");
  
  updatePlotTypes();

  m_interval_le->setDisabled ( true );

  // Add fixed sized column headers to the function params group box
  // This we could not do using the designer.
  m_FunctionParamsListView -> addColumn( QString( "Item" ), 20 );
  m_FunctionParamsListView -> addColumn( QString( "Params" ), 70 );
  m_FunctionParamsListView -> addColumn( QString( "Fixed"  ), 50 );
  m_FunctionParamsListView -> addColumn( QString( "Value" ), 90 );
  m_FunctionParamsListView -> addColumn( QString( "Error" ), 90 );

  connect ( axisWidget1, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( setLowText() ) );
  
  connect ( axisWidget2, SIGNAL ( lowTextReturnPressed() ),
	    this, SLOT ( cutLowText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( setHighText() ) );
  
  connect ( axisWidget2, SIGNAL ( highTextReturnPressed() ),
	    this, SLOT ( cutHighText_returnPressed() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( lowRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( lowSliderReleased() ),
	    this, SLOT ( cutLowSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( highSliderPressed() ),
	    this, SLOT ( setDragOn() ) );

  connect ( axisWidget1, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( setLowRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( lowSliderValueChanged ( int ) ),
	    this, SLOT ( cutLowSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( highRangeDrag() ) );

  connect ( axisWidget2, SIGNAL ( highSliderReleased() ),
	    this, SLOT ( cutHighSlider_sliderReleased() ) );

  connect ( axisWidget1, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( setHighRange ( int ) ) );

  connect ( axisWidget2, SIGNAL ( highSliderValueChanged ( int ) ),
	    this, SLOT ( cutHighSlider_sliderMoved ( int ) ) );

  connect ( axisWidget1, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( axisZoomPanCheckBox_clicked () ) );
  
  connect ( axisWidget2, SIGNAL ( zoomPanCheckBoxClicked () ),
	    this, SLOT ( cutZoomPanCheckBox_clicked () ) );
  
  axisWidget2->setCut ( true );
}

/// Called when low range slider's value changed.
void InspectorBase::setLowRange( int value )
{
}

/// Called when high range slider's value changed.
void InspectorBase::setHighRange( int value )
{
}

/// Called when width slider value is changed.
void InspectorBase::setBinWidth ( int value )
{
}

/// Called when offset slider value is changed.
void InspectorBase::setOffset( int value  )
{
}

/// Called when high range slider is released, thus end of dragging.
void InspectorBase::highRangeDrag()
{
}

/// Called when low range slider is released, thus end of dragging.
void InspectorBase::lowRangeDrag()
{
}

/// Called when width slider is released.  Will set the bin width to
/// value of mouse releaes and set dragging slider off.
void InspectorBase::widthDrag()
{
}

///Called when offset slider is released.
void InspectorBase::offsetDrag()
{
}

/// Called when any slider is pressed.
void InspectorBase::setDragOn ()
{

  m_dragging = true;
  
  if ( ! axisWidget1->isZoomPanChecked() )
    {
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
    }
  else
    {
      // Save current width and position.
      m_autoScale->setChecked ( false );
      autoScale_clicked ();
      
      PlotterBase * plotter = getPlotter ();
      if ( !plotter ) return;    
      const Range & r = plotter->getRange ( m_axis, true );
     m_range.setRange ( r.low(), r.high(), r.pos() );
    }
  
}

/// Called when X-radio button is clicked.
void InspectorBase::setAxisX()
{
  m_axis = Axes::X;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Y-radio button is clicked.    
void InspectorBase::setAxisY()
{
  m_axis = Axes::Y;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when Z-radio button is clicked.
void InspectorBase::setAxisZ()
{
  m_axis = Axes::Z;
  logScale->setEnabled ( true );
  updateAxisTab();
}

/// Called when lowtextbox gets return pressed.  
void InspectorBase::setLowText()
{
  
}

/// Called when hightextbox gets return pressed.
void InspectorBase::setHighText()
{

}

/// Called when widthtextbox gets return pressed.
void InspectorBase::setWidthText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_width_text->text();
  double width = text.toDouble();  

  if ( width == 0 ) return;    // To prevent it from crashing.

  DisplayController::instance() -> setBinWidth ( plotter, m_axis, width );

  updateAxisTab();
}

/// Called when offset text box gets a return pressed event.
void InspectorBase::setOffsetText()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  //Get the string and convert it to double.
  QString text = m_offset_text->text();
  double offset = text.toDouble();  

  int value = static_cast < int > ( 50.0 * offset ) + 49;
  setDragOn ();
  setOffset( value );
  offsetDrag ();

  updateAxisTab ();
}

void InspectorBase::setZRadioButton ( bool enabled )
{
  
  if (!enabled && m_axis == Axes::Z ) 
    {
      radio_button_x->setChecked(true);
      setAxisX();
      logScale->setEnabled (true);
    }
  
  radio_button_z->setEnabled ( enabled );
}

/// Updates the axis tabbed panel.
void InspectorBase::updateAxisTab()
{
}

void InspectorBase::updatePlotTab()
{  
  PlotterBase * plotter = getPlotter ();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  DisplayController * controller = DisplayController::instance ();
  int index = -1;
  if ( plotter != 0 ) index = controller->activeDataRepIndex ( plotter );

  bool yes = index < 0 || text != 0;
  m_plot_symbols->setDisabled ( yes );
  m_plot_title->setDisabled ( yes );
  m_plot_draw->setDisabled ( yes );
  m_plot_color->setDisabled ( yes );
  m_interval_le->setDisabled ( yes );
  m_interval_cb->setDisabled ( yes );
  m_errorBars ->setDisabled ( yes );
  m_pointRepComboBox->setDisabled ( yes );

  if ( yes ) return;

  // Point Reps stuff.

  m_pointRepComboBox->clear();

  DataRep * datarep = plotter->getDataRep ( index );

  yes = datarep -> hasErrorDisplay ();
  m_errorBars -> setEnabled ( yes );

  ProjectorBase * proj = datarep -> getProjector();
  const vector <string> & pointreps = proj -> getPointReps();
  
  for (std::vector<string>::size_type i = 0; i < pointreps.size(); i++ ) {
    m_pointRepComboBox->insertItem ( pointreps[i].c_str() );
  }

  if ( pointreps.empty () == false ) {
    RepBase * rep = datarep->getRepresentation();
    const string & curRep = rep->name();
    m_pointRepComboBox->setCurrentText ( curRep.c_str() );
  }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );
  
  if ( nt && nt -> empty () ) {
    m_plot_symbols->setDisabled ( true );
    m_plot_title->setDisabled ( true );
    m_plot_draw->setDisabled ( true );
    m_plot_color->setDisabled ( true );
    return;
  }

  const std::string & st = plotter->getTitle();
  QString qst ( st.c_str() );
  titleText->setText ( qst );

  m_errorBars->setChecked ( plotter->errorDisplay ( Axes::Y ) );

  ButtonGroup2->setEnabled ( true );

  const string & type = DisplayController::instance() -> pointType ( plotter );
  if ( type.empty () ) {
    ButtonGroup2->setDisabled ( true );
  }
  else {
    if ( type == "Rectangle" )             rectangle->setChecked (true);
    else if ( type == "Filled Rectangle" ) filledRectangle->setChecked (true);
    else if ( type == "+" )                plus->setChecked (true);
    else if ( type == "X" )                cross->setChecked (true);
    else if ( type == "Triangle" )         triangle->setChecked (true);
    else if ( type == "Filled Triangle" )  filledTriangle->setChecked (true);
    else if ( type == "Circle" )           circle->setChecked (true);
    else if ( type == "Filled Circle" )    filledCircle->setChecked (true);
    else
      {
	ButtonGroup2->setDisabled ( true );
      }
  }

  float ptsize =  controller -> pointSize ( plotter );
  m_symbolPointSize -> setText ( QString ("%1").arg (ptsize) );

  const Color & color = plotter->repColor ();
  QColor qcolor ( color.getRed(), color.getGreen(), color.getBlue () );
  m_selectedColor->setPaletteBackgroundColor ( qcolor );

  if ( nt == 0 ) {
    m_interval_cb -> setEnabled ( false );
    m_interval_le -> setEnabled ( false );
    return;
  }

  yes = nt->isIntervalEnabled ();
  m_interval_cb->setChecked ( yes );
  m_interval_le->setEnabled ( yes );

  unsigned int count = nt->getIntervalCount ();
  m_interval_le->setText ( QString ("%1").arg ( count ) );
}

void InspectorBase::autoScale_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( plotter == 0 ) return;
  
  // If the transform be periodic it sets both the offsets to be 0.0
  PeriodicBinaryTransform *tp =
    dynamic_cast< PeriodicBinaryTransform* > ( plotter->getTransform() );
  if ( tp != 0 )
    {
      tp->setXOffset( 0.0 );
      tp->setYOffset( 0.0 );
    }

  // For all transforms sets autoranging of the axis active
  bool checked = m_autoScale->isChecked();
  plotter->setAutoRanging( m_axis, checked );
  
  updateAxisTab();  
}

void InspectorBase::logScale_clicked()
{
  PlotterBase * plotter = getPlotter();
  if ( !plotter ) return;
  
  bool checked = logScale->isChecked();
  DisplayController::instance() -> setLog ( plotter, m_axis, checked );

  checked = m_autoScale->isChecked();
  plotter->setAutoRanging ( m_axis, checked );
  
  updateAxisTab();
}

/** Response to request of color of DataRep. */
void InspectorBase::colorSelect_clicked()
{
}


void InspectorBase::titleText_returnPressed()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  std::string s( (titleText->text()).latin1() );
  plotter->setTitle ( s );
}

void InspectorBase::errorBars_toggled( bool )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  bool checked = m_errorBars->isChecked();
  DisplayController * controller = DisplayController::instance ();

  controller -> setErrorDisplayed ( plotter, Axes::Y, checked );
}

/** Responds to user selection of one of the plotting symbols check
    boxes. */
void InspectorBase::symbolTypeButtonGroup_clicked ( int id )
{
  
}

/** Responds to user changing plot symbol point size. */
void InspectorBase::symbolPointSize_returnPressed()
{

}

/** Responds to change in axis binding on existing plotter. */
void InspectorBase::axisLabelChanged ( const QString & label, 
				       const QString & axisName )
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  const std::string strAxisName( axisName.latin1() );
 
  const std::string strLabel( label.latin1() );
  DisplayController::instance() 
    -> setAxisBinding ( plotter, strAxisName, strLabel );
}

void InspectorBase::updatePlotTypes ()
{
  const vector < string > & dataRepNames 
    = DisplayController::instance() -> getDisplayTypes ();
  if ( dataRepNames.empty () ) return;

  m_availPlotTypes->clear();

  vector < string > ::const_iterator first = dataRepNames.begin ();
  while ( first != dataRepNames.end() ) {
    const string & name = *first++;
    if ( name.find ( "Static" ) != string::npos ) continue;
    m_availPlotTypes->insertItem ( name.c_str() );
  }
  m_availPlotTypes->setCurrentItem ( 2 ); //Histogram

  newPlotButton->setEnabled( true );
}

/** Responds to "New Plot" button being clicked. */
void InspectorBase::newPlotButton_clicked()
{

}

void InspectorBase::addDataRepButton_clicked()
{
}


void InspectorBase::selectedCutsRadioButton_toggled ( bool selected )
{

}

void InspectorBase::allCutsRadioButton_toggled ( bool selected )
{
  // implemented in derived class
}

/** Responds to return in cut high text. */
void InspectorBase::cutHighText_returnPressed ()
{

}

/** Responds to return in cut low text. */
void InspectorBase::cutLowText_returnPressed ()
{

}

void InspectorBase::cutHighSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  
  axisWidget2->processHighSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderMoved ( int value )
{
  CutPlotter * cd = getSelectedCut();
  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );

  axisWidget2->processLowSliderMoved ( value, currentRange, fullRange );
  
  cd->setCutRange ( currentRange );
}

void InspectorBase::cutLowSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processLowSliderReleased ( fullRange );
}

void InspectorBase::cutHighSlider_sliderReleased()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processHighSliderReleased ( fullRange );
}

void InspectorBase::colorSelect_2_clicked()
{
  CutPlotter * cd = getSelectedCut();

  const Color & rep_color = cd->getCutColor ();
  QColor color ( rep_color.getRed(),
		 rep_color.getGreen(),
		 rep_color.getBlue () );

  color = QColorDialog::getColor ( color );
  if ( color.isValid() == false ) return;

  Color c ( color.red(), color.green(), color.blue() );
  cd->setCutColor ( c );
}

void InspectorBase::cutInvertPushButton_clicked()
{
  CutPlotter * cd = getSelectedCut();
  cd->toggleInverted ( );
}

void InspectorBase::cutZoomPanCheckBox_clicked()
{
  CutPlotter * cd = getSelectedCut();
  if ( cd == 0 ) return;

  bool yes = axisWidget2 -> isZoomPanChecked ();
  CutController * controller = CutController::instance();
  controller -> setZoomPan ( cd, yes );

  Range currentRange = cd->cutRange();
  const Range & fullRange = cd->getRange ( Axes::X, false );
  axisWidget2->processZoomPanCheckBoxClicked ( currentRange, fullRange );
}

/* virtual and implemented in derived class. MS VC++ 6. insists on
   returning something. */
CutPlotter * InspectorBase::getSelectedCut ()
{
  return 0;
}

void InspectorBase::cutAddPushButton_clicked()
{
  // Take the selected cut from cutlistcombobox and add it to selected
  // plotter on canvas.

  // Find the selected cutplotter.
  
  CutPlotter * cd = getSelectedCut();

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Add the cut to the plotter.
  CutController::instance() -> addCut ( cd, plotter );  
  
}

/** Responds to click on Remove pushbutton. */
void InspectorBase::cutRemovePushButton_clicked()
{

  // Take the selected cut from cutlistcombobox and remove it from the selected
  // plotter on canvas.

  // Find the selected Plotter.

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;

  // Remove the cut from the plotter.

  // Find the selected cutplotter.
  CutPlotter * cd = getSelectedCut();
  CutController::instance() -> removeCut ( cd, plotter );  

  if ( m_selectedPlotRadioButton->isChecked() ) {
    updateCutsTab ();
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~~~ FUNCTIONS TAB PANE STUFF ~~~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/** @todo No need to initialize function selection box on each update. 
 */
void InspectorBase::updateFunctionsTab()
{
  PlotterBase * plotter = getPlotter();
  TextPlotter * text = dynamic_cast < TextPlotter * > ( plotter );
  if ( plotter == 0 || text != 0 )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      m_resid->setDisabled ( true );
      return;
    }

  const NTuple * nt = DisplayController::instance()->getNTuple ( plotter );

  if ( nt && nt -> empty () )
    {
      m_func_new->setDisabled ( true );
      m_func_applied->setDisabled ( true );
      m_func_parms->setDisabled ( true );
      return;
    }

  // Update new functions section //
  //------------------------------//
  newFunctionsAddButton->setEnabled ( true );
  m_func_new->setEnabled ( true );
  
  // Get available function names from the function factory and put them
  // in the newFunctionsComboBox.
  const vector < string > & names = FunctionFactory::instance() -> names ();
  int current = newFunctionsComboBox->currentItem ();
  newFunctionsComboBox->clear();
  
  for ( unsigned int i = 0; i < names.size(); i++)
    if ( names[i] != "Linear Sum" )
      newFunctionsComboBox->insertItem ( names[i].c_str() );
  
  newFunctionsComboBox->setCurrentItem(current);
  newFunctionsComboBox->setEnabled ( true );
  
  // Update functionsAppliedComboBox. //
  //----------------------------------//
  bool to_enable = false;
  DisplayController * controller = DisplayController::instance ();

  int index = controller -> activeDataRepIndex ( plotter );
  FunctionController * f_controller = FunctionController::instance ();
  if ( index >= 0 )
    {
      DataRep * datarep = plotter ->getDataRep ( index );
      const vector < string > & fnames
	= f_controller -> functionNames ( plotter, datarep );

      if ( fnames.size() != 0 )
	{
	  to_enable = true;
	  functionsAppliedComboBox->clear();
	  m_functionIndexMap.clear();
	  
	  for ( unsigned i = 0; i < fnames.size(); i++)
	    {
	      if ( fnames[i] != "Linear Sum" )
		{
		  functionsAppliedComboBox->insertItem ( fnames[i].c_str() );
		  m_functionIndexMap.push_back ( i );
		}
	    } 
	  functionsAppliedComboBox->setCurrentItem (0);
	}
    }
  
  m_func_applied->setEnabled ( to_enable );

  if ( to_enable )
    {
      // get to original string to avoid possible error in conversion to
      // UNICODE and back.
      const vector < string > & fitters = f_controller -> getFitterNames ();
      const string & name = f_controller -> getFitterName ( plotter );
      
      // Didn't use find because want index
      for ( unsigned int i = 0; i < fitters.size(); i++ ) 
	if ( name == fitters[i] )
	  {
	    m_fitter_names -> setCurrentItem ( i );
	    break;
	  }
    }
  
  m_resid->setEnabled ( to_enable );

  // Update function parameters tab //
  //--------------------------------//
  
  // Set Parameters in list view as well as in line editor and
  // the check box. Focus is set to the current selected item
  // or in case none is selected 1st item.
  setParameters ( index, plotter );
  
  // Set the slider to be in the center
  m_FunctionParamsSlider -> setValue(50);
  
}


void InspectorBase::setParameters ( int index, PlotterBase * plotter )
{
  m_FunctionParamsListView -> clear();
  m_FunctionParamsCheckBox -> setChecked( false );
  m_FunctionParamsLineEdit -> clear();
  
  FunctionController * controller = FunctionController::instance ();
  if ( ! ( controller -> hasFunction ( plotter ) ) )
    {
      m_func_parms -> setDisabled ( true );
      return;
    }

  if ( index < 0 ) return;
  
  m_func_parms -> setEnabled ( true );
  
  DataRep * datarep = plotter -> getDataRep ( index );
  const vector < string > & fnames
    = controller -> functionNames ( plotter, datarep );

  int count = 0;
  
  for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
    if ( fnames [ findex ] != "Linear Sum" )
      {
	const vector < string > & paramNames 
	  = controller -> parmNames  ( plotter, findex );
	const vector < bool > & fixedFlags 
	  = controller -> fixedFlags ( plotter, findex );
	const vector < double > & parameters 
	  = controller -> parameters ( plotter, findex );
	const vector < double > & principleErrors
	  = controller -> principleErrors ( plotter, findex );
	
	for( unsigned int pindex = 0; pindex < paramNames.size(); pindex++ )
	  {
	    QListViewItem * item
	      = new QListViewItem( m_FunctionParamsListView );
	    
	    item -> setText( 0, QString( "%1" ).arg( count + pindex + 1 ) );
	    item -> setText( 1, QString( "%1" ).arg( paramNames[ pindex ] ) );
	    item -> setText( 2, QString( "%1" ).arg( fixedFlags[ pindex ] ) );
	    item -> setText( 3, QString( "%1" ).arg( parameters[ pindex ] ) );
	    item -> setText( 4, QString( "%1" ).arg( principleErrors[pindex] ));
	    item -> setText( 5, QString( "%1" ).arg( findex ) );
	    item -> setText( 6, QString( "%1" ).arg( pindex ) );
	    item -> setText( 7, QString( "%1" ).arg( index ) );
	    
	    m_FunctionParamsListView -> insertItem( item );
	    
	  }
	count += paramNames.size();
      }
  
  m_FunctionParamsListView -> setAllColumnsShowFocus ( true );
  QListViewItem * firstItem = m_FunctionParamsListView -> firstChild ();
  m_FunctionParamsListView -> setSelected ( firstItem, true );
  m_FunctionParamsListView -> setCurrentItem ( firstItem );
  
  QString fixed = firstItem -> text( 2 );
  unsigned int fixedFlag = fixed.toUInt();
  
  if( fixedFlag )
    m_FunctionParamsCheckBox -> setChecked( true );
  else
    m_FunctionParamsCheckBox -> setChecked( false );
  
  m_FunctionParamsLineEdit -> setText ( firstItem -> text( 3 ) );
  
}


void InspectorBase::functionParamsListViewCurrentChanged( QListViewItem * )
{

}


void InspectorBase::functionParamsCheckBoxToggled( bool )
{

}


void InspectorBase::functionParamsLineEditReturnPressed()
{

}

void InspectorBase::functionParamsSliderSliderReleased()
{

}


void InspectorBase::functionParamsSliderSliderMoved( int )
{

}


void InspectorBase::functionParamsSliderSliderPressed()
{

}

void InspectorBase::functionsRemoveButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  // Get the index of the function selected in functionsAppliedComboBox.
  QString qstr = functionsAppliedComboBox->currentText();
  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox -> currentItem();
  int funcindex = m_functionIndexMap[item];

  // Remove the function.
  FunctionController::instance() -> removeFunction ( plotter, funcindex );
  
  // Set Active Plot.
  if ( plotter->activePlotIndex ( ) != 0 )
    plotter->setActivePlot ( -1, true );
  else
    plotter->setActivePlot ( 0, true );
  
  // Update the rest.
  updateFunctionsTab();
  updateErrorEllipseTab();
}


/** #todo Replace cout message with dialog. */
void InspectorBase::functionsFitToDataButton_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }

  FunctionController::instance() -> saveParameters ( plotter );

  // Find the index of the function selected.

  QString qstr = functionsAppliedComboBox->currentText();

  if ( !qstr ) return;
  
  int item = functionsAppliedComboBox->currentItem();
  int funcindex = m_functionIndexMap[item];

  // Fit the function, check for error.
  FunctionController * fcnt = FunctionController::instance();
  assert( fcnt );
   
  int ok = fcnt-> fitFunction ( plotter, funcindex );

  if ( ! ok )
    cout << "Fit Function failed to converge" << endl;

  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
  
  setParameters ( index, plotter );
}

void InspectorBase::functionsResetButton_clicked()
{
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return ;  

  if ( ! ( FunctionController::instance() -> hasFunction ( plotter ) ) ) {
    return;
  }
  
  FunctionController::instance() -> restoreParameters ( plotter );
  
  // Set the parameters
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );
    
  setParameters ( index, plotter );
}

/** Returns the selected plotter.  Returns the selected plotter upon
    which updates and controls are to be attached. */
PlotterBase * InspectorBase::getPlotter ()
{
  return 0;
}


/* Updates the tabbed pane that contains the ellipse options*/
void InspectorBase::updateErrorEllipseTab()
{
  // Check if there is plotter.
  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  
  // Check if there is a function attached to this plotter.
  FunctionController * fcontroller = FunctionController::instance ();
  if ( ! ( fcontroller -> hasFunction ( plotter ) ) ) 
    return;
  
  // Get indec of the active data rep
  DisplayController * dcontroller = DisplayController::instance ();
  int index = dcontroller -> activeDataRepIndex ( plotter );

  // Get list of functions with this datarep
//   DataRep * datarep = plotter -> getDataRep ( index );
//   const vector < string > & fnames
//     = fcontroller -> functionNames ( plotter, datarep );
    
  // Update the error ellipsoid tab by putting them in the combo boxes //
//   m_ComboBoxEllipsoidParamX -> clear();
//   m_ComboBoxEllipsoidParamY -> clear();
  
//   for ( unsigned int findex = 0; findex < fnames.size(); findex++ )
//     if ( fnames [ findex ] != "Linear Sum" )
//       {
// 	const vector < string > & paramNames 
// 	  = fcontroller -> parmNames  ( plotter, findex );
// 	for ( unsigned int i = 0; i < paramNames.size(); i++ )
// 	  {
// 	    m_ComboBoxEllipsoidParamX -> insertItem ( paramNames[i].c_str() );
// 	    m_ComboBoxEllipsoidParamY -> insertItem ( paramNames[i].c_str() );
// 	  }
//       }
}

/* Updates the tabbed pane that contains the summary options. */
void InspectorBase::updateSummaryTab()
{

}


/** Responds to a click on one of the fix parameter check boxes on the
    Function tabbed panel. */
/** Responds to a click on "New" button on the Cut tabbed panel. */
void InspectorBase::cutNew()
{

}

/** Responds to a click on "New" button on the Summary tabbed panel. */
void InspectorBase::summaryNew()
{

}

/** Responds to a click on the "add" button on the Function tabbed panel. */
void InspectorBase::functionAdd()
{

}



/** Responds to change of selected cut by updating the cut controls' values.*/
void InspectorBase::selCutChanged()
{

}



/** Updates the widgets on the Cuts tabbed panel.  

@attention This member function declared as slot with Qt Designer to
be compatible with Qt < 3.1.
*/
void InspectorBase::updateCutsTab()
{

}


void InspectorBase::intervalStateChanged( bool )
{

}

void InspectorBase::intervalTextChanged( const QString & )
{

}


void InspectorBase::contourSlider_valueChanged ( int val )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContoursTextBox->setText ( QString("%1").arg ( val ) );

  datarep->notifyObservers();

}

void InspectorBase::contourTextBox_returnPressed()
{ 

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;
  
  QString text = m_numContoursTextBox->text();
  int val = text.toInt();

  if ( val < 1 || val > 100 ) {
    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );    
    return;
  }
  
  contourRep->setUsingUserValues ( false );
  contourRep->setNumContours ( val );
  m_numContourSlider->setValue ( val );

  datarep->notifyObservers();

}

void InspectorBase::axisZoomPanCheckBox_clicked()
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;

  plotter->setAutoRanging ( m_axis, false );
  const Range & r = plotter->getRange ( m_axis, true );

  m_autoScale->setChecked ( false );

  if ( axisWidget1->isZoomPanChecked() ) {    
    m_zoompan[plotter] = true;
  }

  else {
    
    std::map < const PlotterBase *, bool >::const_iterator it
      = m_zoompan.find ( plotter );
    if ( it != m_zoompan.end () ) {
      m_zoompan[plotter] = false;
    }
    
  }

  axisWidget1->processZoomPanCheckBoxClicked ( r, r );

}

void InspectorBase::contourRadioButton1_toggled( bool )
{

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

  if ( contourRadioButton1->isChecked() ) {
    
    m_numContourSlider->setEnabled ( true );
    m_numContoursTextBox->setEnabled ( true );
    m_numContoursLabel->setEnabled ( true );
    m_contourLevelsTextBox->setEnabled ( false );

    int num = contourRep->getNumContours ();
    m_numContourSlider->setValue ( num );
    m_numContoursTextBox->setText ( QString ("%1").arg ( num ) );

    contourSlider_valueChanged ( num );

  }
    
  else {

    m_numContourSlider->setEnabled ( false );
    m_numContoursTextBox->setEnabled ( false );
    m_numContoursLabel->setEnabled ( false );
    m_contourLevelsTextBox->setEnabled ( true );

    contourLevelsTextBox_returnPressed();
    
  }

}


void InspectorBase::contourLevelsTextBox_returnPressed()
{
  if ( contourRadioButton2->isChecked () == false ) return;

  PlotterBase * plotter = getPlotter ();
  if ( !plotter ) return;
  DisplayController * controller = DisplayController::instance ();
  int index = controller->activeDataRepIndex ( plotter );
  DataRep * datarep = plotter->getDataRep ( index );

  RepBase * rep = datarep->getRepresentation();
  ContourPointRep * contourRep = dynamic_cast < ContourPointRep * > ( rep );

  if ( !contourRep ) return;

   const QString qstr1 = m_contourLevelsTextBox->text();
   if ( qstr1.isEmpty () ) return;
   const QString qstr2 = qstr1.simplifyWhiteSpace();

   QTextIStream * stream = new QTextIStream ( &qstr2 );

   vector < double > values;
   double prev = 0, cur = 0;
   bool first = true;
  
   while ( !stream->atEnd() ){

    QString strval;
    (*stream) >> strval;
    bool ok = true;
    cur = strval.toDouble ( &ok );

    // Check if its a legal double value.

    if ( !ok ) {
      contourError();
      return;
    }

    // Check sorted.
    
    if ( first ) {
      first = false;
    }
    else {
      if ( cur <= prev ) {
	contourError();
	return;
      }
    }

    // Push value.

    values.push_back ( cur );
    prev = cur;
    
   }
   
   contourRep->setContourValues ( values, datarep->getProjector() );
   datarep->notifyObservers();
   
}

void InspectorBase::contourError()
{
  const QString message =
    "Invalid Input String. Please check that\n"
    "1. The string contains only numbers separated by white spaces, and,\n"
    "2. The numbers are in increasing order without any duplicates.\n";
  QMessageBox::critical ( this, // parent
			  "Invalid Input String", // caption
			  message,
			  QMessageBox::Ok,
			  QMessageBox::NoButton,
			  QMessageBox::NoButton );
}

void InspectorBase::pointRepComboBox_activated ( const QString & qstr )
{
}

/** Creates a display showing the residuals of the function. 
 */
void InspectorBase::createResiduals()
{
}

/** The slot that receives the signal from the fitter names
    selector. 
*/
void InspectorBase::fitterNamesActivated(int)
{
}



/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It plot a new error plot with the two parameters selected */
void InspectorBase::pushButtonNewErrorPlotClicked()
{                  

}

/** The slot that recieves the signal when the PushButtonNewErrorPlot
    is clicked. It just refreshes the current plot with the two parameters
    selected 
*/
void InspectorBase::pushButtonRefreshErrorPlotClicked()
{
  
}

/** Responds to <b> Create ntuple </b> button. 
 */
void InspectorBase::dataCreateNTuple()
{

}


void InspectorBase::dataTupleNameChanged( const QString & )
{

}

/** Responds to m_all_ntuples QComboBox being activated. Reponds to
    all ntuples combo box being activated by either updating the axis
    binding options or changing the name of the NTuple.
 */
void InspectorBase::allNtupleComboActivated( const QString & )
{

}

/** Responds to m_avaiPlotTypes being activated. 
 */
void InspectorBase::availPlotTypesActivated( const QString & )
{

}

/** Responds to change in all ntuple selection. 
 */
void InspectorBase::dataNTupleSelChanged( int item )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamX
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the X axis against which the Confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamYHighlighted( int )
{

}


/** The slot that recieves the signal when ComboBoxEllipsoidParamY
    (which is in the Confidence ellipsoid Group Box) is highlighted.
    It sets the paramter along the Y axis against which the confidence
    ellipsoid is to be plotted */
void InspectorBase::comboBoxEllipsoidParamXHighlighted( int )
{

}
