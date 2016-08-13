#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDateTime>

#include "rlistrings.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "MainWindow construction start";
  ui->setupUi(this);

  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "RadarDS init start";

  _radar_ds = new RadarDataSource();
  _chart_mngr = new ChartManager();

  ui->wgtRLIDisplay->setChartManager(_chart_mngr);

  QStringList args = qApp->arguments();
  QRegExp     rx("--radar-device");
  int argpos = args.indexOf(rx);

  if((argpos >= 0) && (argpos < args.count() - 1))
      _radar_ds->start(args.at(argpos + 1).toStdString().c_str());
  else
      _radar_ds->start();

  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "RadarDS init finish";

  _gain_ctrl = new ValueBarController(RLIStrings::nGain, QPoint(5, 5), 8, 0, this);
  connect(ui->wgtRLIControl, SIGNAL(gainChanged(int)), _gain_ctrl, SLOT(onValueChanged(int)));

  _water_ctrl = new ValueBarController(RLIStrings::nWave, QPoint(5, 5+23+4), 8, 0, this);
  connect(ui->wgtRLIControl, SIGNAL(waterChanged(int)), _water_ctrl, SLOT(onValueChanged(int)));

  _rain_ctrl = new ValueBarController(RLIStrings::nRain, QPoint(5, 5+2*(23+4)), 8, 0, this);
  connect(ui->wgtRLIControl, SIGNAL(rainChanged(int)), _rain_ctrl, SLOT(onValueChanged(int)));

  _apch_ctrl = new ValueBarController(RLIStrings::nAfc, QPoint(5, 5+3*(23+4)), 8, 0, this);
  _radiation_ctrl = new ValueBarController(RLIStrings::nEmsn, QPoint(5+12*8+60+5, 5), 9, -1, this);

  _curs_ctrl = new CursorController(this);
  connect(ui->wgtRLIDisplay, SIGNAL(cursor_moved(float,float)), _curs_ctrl, SLOT(cursor_moved(float,float)));

  _clck_ctrl = new ClockController(this);
  connect(ui->wgtRLIDisplay, SIGNAL(per_second()), _clck_ctrl, SLOT(second_changed()));

  _scle_ctrl = new ScaleController(this);

  _lbl1_ctrl = new LabelController(RLIStrings::nNord, 120, QPoint(246, 45), false, this);
  _lbl2_ctrl = new LabelController(RLIStrings::nRm, 120, QPoint(246, 70), false, this);
  _lbl3_ctrl = new LabelController(RLIStrings::nWstab, 120, QPoint(246, 95), false, this);

  _crse_ctrl = new CourseController(this);

  _pstn_ctrl = new PositionController(this);
  _blnk_ctrl = new BlankController(this);

  connect(ui->wgtRLIDisplay, SIGNAL(initialized()), this, SLOT(onRLIWidgetInitialized()));
  startTimer(33);

  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "MainWindow construction finish";
}

MainWindow::~MainWindow() {
  delete ui;

  delete _radar_ds;
  delete _chart_mngr;

  delete _gain_ctrl;
  delete _water_ctrl;
  delete _rain_ctrl;
  delete _apch_ctrl;
  delete _radiation_ctrl;

  delete _scle_ctrl;

  delete _lbl1_ctrl;
  delete _lbl2_ctrl;
  delete _lbl3_ctrl;

  delete _crse_ctrl;

  delete _pstn_ctrl;
  delete _blnk_ctrl;

  delete _curs_ctrl;
  delete _clck_ctrl;
}

void MainWindow::onRLIWidgetInitialized() {
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "Connect radar to datasource";
  connect(_radar_ds, SIGNAL(updateData(uint,uint,float*,float*))
        , ui->wgtRLIDisplay->radarEngine(), SLOT(updateData(uint,uint,float*,float*)));


  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "Setup InfoBlocks";
  setupInfoBlock(_gain_ctrl);
  setupInfoBlock(_water_ctrl);
  setupInfoBlock(_rain_ctrl);
  setupInfoBlock(_apch_ctrl);
  setupInfoBlock(_radiation_ctrl);

  setupInfoBlock(_curs_ctrl);
  setupInfoBlock(_clck_ctrl);

  setupInfoBlock(_pstn_ctrl);
  setupInfoBlock(_blnk_ctrl);

  setupInfoBlock(_crse_ctrl);

  setupInfoBlock(_scle_ctrl);
  setupInfoBlock(_lbl1_ctrl);
  setupInfoBlock(_lbl2_ctrl);
  setupInfoBlock(_lbl3_ctrl);

  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "Setup rliwidget as control reciever";
  ui->wgtRLIControl->setReciever(ui->wgtRLIDisplay);

  connect(_chart_mngr, SIGNAL(new_chart_available(QString)), ui->wgtRLIDisplay, SLOT(new_chart(QString)));
  _chart_mngr->loadCharts();

  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << ": " << "Setup menu signals";
  connect( ui->wgtRLIDisplay->menuEngine(), SIGNAL(languageChanged(QByteArray))
         , ui->wgtRLIDisplay->infoEngine(), SLOT(onLanguageChanged(QByteArray)));

  connect( ui->wgtRLIDisplay->menuEngine(), SIGNAL(languageChanged(QByteArray))
         , ui->wgtRLIDisplay->menuEngine(), SLOT(onLanguageChanged(QByteArray)));
}

void MainWindow::setupInfoBlock(InfoBlockController* ctrl) {
  InfoBlock* blck = ui->wgtRLIDisplay->infoEngine()->addInfoBlock();
  ctrl->setupBlock(blck, ui->wgtRLIDisplay->size());

  connect(ctrl, SIGNAL(setRect(int, QRect)), blck, SLOT(setRect(int, QRect)));
  connect(ctrl, SIGNAL(setText(int, int, QByteArray)), blck, SLOT(setText(int, int, QByteArray)));
  connect(ui->wgtRLIDisplay, SIGNAL(resized(QSize)), ctrl, SLOT(onResize(QSize)));
}

void MainWindow::resizeEvent(QResizeEvent* e) {
  Q_UNUSED(e);
  QRect rli_geom = ui->wgtRLIDisplay->geometry();

  rli_geom.setWidth(4 * (rli_geom.width() / 4));
  rli_geom.setHeight(3 * (rli_geom.height() / 3));

  if (rli_geom.height() > 3 * (rli_geom.width() / 4))
    rli_geom.setHeight(3 * (rli_geom.width() / 4));
  else
    rli_geom.setWidth(4 * (rli_geom.height() / 3));

  ui->wgtRLIDisplay->setGeometry(rli_geom);
}

void MainWindow::timerEvent(QTimerEvent* e) {
  Q_UNUSED(e);
  ui->wgtRLIDisplay->update();
}

void MainWindow::on_btnClose_clicked() {
  close();
}
