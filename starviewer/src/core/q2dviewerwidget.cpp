/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "q2dviewerwidget.h"
#include "volume.h"
#include "logging.h"
#include "statswatcher.h"
#include "synchronizetool.h"
#include "toolproxy.h"

#include <QAction>

namespace udg {

Q2DViewerWidget::Q2DViewerWidget(QWidget *parent)
 : QFrame(parent), m_mainVolume(0)
{
    setupUi(this);
    setAutoFillBackground(true);

    // Creació de l'acció del boto de sincronitzar.
    m_synchronizeButtonAction = new QAction(0);
    m_synchronizeButtonAction->setText(tr("Synchronize tool"));
    m_synchronizeButtonAction->setStatusTip(tr("Enable/Disable Synchronize tool"));
    m_synchronizeButtonAction->setIcon(QIcon(":/images/synchronize.png"));
    m_synchronizeButtonAction->setCheckable(true);
    m_synchronizeButton->setDefaultAction(m_synchronizeButtonAction);
    m_synchronizeButton->setEnabled(false);

    createConnections();
    m_viewText->setText(QString());

    this->setSliderBarWidgetsEnabledFromViewerStatus();

    m_statsWatcher = new StatsWatcher("Q2DViewerWidget", this);
    m_statsWatcher->addClicksCounter(m_synchronizeButton);
    m_statsWatcher->addSliderObserver(m_slider);
}

Q2DViewerWidget::~Q2DViewerWidget()
{
}

void Q2DViewerWidget::updateViewerSliceAccordingToSliderAction(int action)
{
    switch (action)
    {
        case QAbstractSlider::SliderMove:
        case QAbstractSlider::SliderPageStepAdd:
        case QAbstractSlider::SliderPageStepSub:
            m_2DView->setSlice(m_slider->sliderPosition());
            break;

        default:
            break;
    }
}

void Q2DViewerWidget::createConnections()
{
    connect(m_slider, SIGNAL(actionTriggered(int)), SLOT(updateViewerSliceAccordingToSliderAction(int)));
    connect(m_2DView, SIGNAL(sliceChanged(int)), m_slider, SLOT(setValue(int)));
    connect(m_2DView, SIGNAL(sliceChanged(int)), SLOT(updateProjectionLabel()));
    connect(m_2DView, SIGNAL(viewChanged(int)), SLOT(resetSliderRangeAndValue()));
    connect(m_2DView, SIGNAL(slabThicknessChanged(int)), SLOT(resetSliderRangeAndValue()));

    // HACK amb això conseguim que quan es varïi el valor de la llesca amb l'slider, el viewer es marqui com a seleccionat
    connect(m_slider, SIGNAL(sliderPressed()), SLOT(emitSelectedViewer()));

    connect(m_2DView, SIGNAL (selected()), SLOT(emitSelectedViewer()));
    connect(m_2DView, SIGNAL(volumeChanged(Volume *)), SLOT(updateInput(Volume *)));

    connect(m_2DView, SIGNAL(slabThicknessChanged(int)), SLOT(updateSlider()));
    connect(m_synchronizeButtonAction, SIGNAL(toggled(bool)), SLOT(enableSynchronization(bool)));

    connect(m_2DView, SIGNAL(viewerStatusChanged()), SLOT(setSliderBarWidgetsEnabledFromViewerStatus()));
}

void Q2DViewerWidget::updateProjectionLabel()
{
    m_viewText->setText(m_2DView->getCurrentPlaneProjectionLabel());
}

void Q2DViewerWidget::setSliderBarWidgetsEnabledFromViewerStatus()
{
    if (m_2DView->getViewerStatus() == Q2DViewer::VisualizingVolume)
    {
        this->setSliderBarWidgetsEnabled(true);
    }
    else
    {
        this->setSliderBarWidgetsEnabled(false);
    }
}

void Q2DViewerWidget::setInput(Volume *input)
{
    m_mainVolume = input;
    m_2DView->setInput(input);
    updateProjectionLabel();
}

void Q2DViewerWidget::setInputAsynchronously(Volume *input, QViewerCommand *command)
{
    if (input)
    {
        m_mainVolume = input;
        m_2DView->setInputAsynchronously(input, command);
        updateProjectionLabel();
    }
}

void Q2DViewerWidget::updateInput(Volume *input)
{
    m_mainVolume = input;
    m_synchronizeButton->setEnabled(true);
    m_slider->setMaximum(m_2DView->getMaximumSlice());
}

void Q2DViewerWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
    Q_UNUSED(mouseEvent);
    emitSelectedViewer();
}

void Q2DViewerWidget::emitSelectedViewer()
{
    if (!m_2DView->isActive())
    {
        emit selected(this);
    }
}

void Q2DViewerWidget::setSelected(bool option)
{
    // Per defecte li donem l'aspecte de background que té l'aplicació en general
    // TODO podríem tenir a nivell d'aplicació centralitzat el tema de
    // gestió de les diferents paletes de l'aplicació
    QBrush brush = QApplication::palette().window();
    if (option)
    {
		// Si seleccionem el widget, li canviem el color de fons
		brush.setColor(QColor(85, 160, 255));	
    }
	QPalette palette = this->palette();
	palette.setBrush(QPalette::Active, QPalette::Window, brush);
	setPalette(palette);
    m_2DView->setActive(option);
}

Q2DViewer* Q2DViewerWidget::getViewer()
{
    return m_2DView;
}

bool Q2DViewerWidget::hasPhases()
{
    int phases = 0 ;
    if (m_mainVolume)
    {
        phases = m_mainVolume->getNumberOfPhases();
    }
    return (phases > 1) ;
}

void Q2DViewerWidget::setDefaultAction(QAction *synchronizeAction)
{
    m_synchronizeButton->setDefaultAction(synchronizeAction);
}

void Q2DViewerWidget::enableSynchronization(bool enable)
{
    if (m_synchronizeButtonAction->isChecked() != enable)
    {
        // Ens han invocat el mètode directament, no s'ha fet clicant el botó
        // Això farà invocar aquest mètode de nou passant per "l'else"
        m_synchronizeButtonAction->setChecked(enable);
    }
    else
    {
        SynchronizeTool *synchronizeTool = dynamic_cast<SynchronizeTool *>(getViewer()->getToolProxy()->getTool("SynchronizeTool"));
        if (synchronizeTool)
        {
            synchronizeTool->setEnabled(enable);
        }
        else
        {
            DEBUG_LOG("El viewer no té registrada l'eina de sincronització, per tant no es pot activar la sincronització");
            // TODO deixar el botó en estat "un-checked"?
        }
    }
}

void Q2DViewerWidget::updateSlider()
{
    m_slider->setValue(m_2DView->getCurrentSlice());
}

void Q2DViewerWidget::resetSliderRangeAndValue()
{
    m_slider->setMaximum(m_2DView->getMaximumSlice() - m_2DView->getSlabThickness()+ 1);
    m_slider->setValue(m_2DView->getCurrentSlice());
}

void Q2DViewerWidget::setSliderBarWidgetsEnabled(bool enabled)
{
    m_slider->setEnabled(enabled);
    m_synchronizeButtonAction->setEnabled(enabled);
    m_viewText->setEnabled(enabled);
}
}
