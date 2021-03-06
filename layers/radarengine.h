#ifndef RADARENGINE_H
#define RADARENGINE_H

#include <QTime>
#include <QColor>
#include <QVector2D>

#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLFramebufferObject>
#include <QtOpenGL/QGLShaderProgram>

// Класс для расчета радарной палитры
class RadarPalette {
public:
  RadarPalette();

  void setRgbVar(int var);
  void setBrightness(int br);

  inline float* getPalette() { return &palette[0][0]; }

private:
  // Расчёт зависимости RGBкодов цвета от амплитуды входного сигнала
  void updatePalette();

  // Параметры:
  int rgbRLI_Var;         //Текущая палитра (день/ночь)
  int brightnessRLI;      //Яркость РЛИ 0..255

  // Текущая палитра
  float palette[16][3];

  // Описание палитры РЛИ
  typedef struct rgbRLI_struct {
    unsigned char Rbg,Gbg,Bbg;        //RGB фона
    unsigned char R01,G01,B01;        //RGB для 1й градации РЛИ
    unsigned char R08,G08,B08;        //RGB для 8й градации РЛИ
    unsigned char R15,G15,B15;        //RGB для 15й градации РЛИ
    unsigned char Rtk,Gtk,Btk;        //RGB следов
    float gamma01_08;                 //линейность яркости от 1й до 8й градации РЛИ
    float gamma08_15;                 //линейность яркости от 8й до 15й градации РЛИ
  } rgbRLI_struct;
};


// Класс для отрисовки радарного круга
class RadarEngine : public QObject, protected QGLFunctions {
  Q_OBJECT
public:
  explicit RadarEngine  (uint pel_count, uint pel_len);
  virtual ~RadarEngine  ();

  // Инициализация OpenGL структур
  bool init           (const QGLContext* context);

  void resizeData     (uint pel_count, uint pel_len);
  void resizeTexture  (uint radius);

  void shiftCenter    (QPoint center);
  void shiftNorth     (uint north);

  inline uint  getSize() const          { return 2*_radius + 1; }
  inline uint  getTextureId() const     { return _fbo->texture(); }

  inline uint  pelengCount() const      { return _peleng_count; }

public slots:
  void clearTexture();
  void clearData();

  void onBrightnessChanged(int br);

  void updateTexture();
  void updateData(uint offset, uint count, GLfloat* amps);

private:
  void initShader();
  void drawPelengs(uint first, uint last);

  void fillCoordTable();

  bool _initialized;
  bool _has_data;

  // Radar parameters
  QPoint  _center;
  uint    _north;
  uint    _radius, _peleng_count, _peleng_len;

  std::vector<GLfloat> _coord_table;

  bool  _draw_circle;
  uint  _last_drawn_peleng, _last_added_peleng;

  // Framebuffer vars
  QGLFramebufferObjectFormat _fbo_format;
  QGLFramebufferObject* _fbo;
  QGLShaderProgram* _prog;

  enum { ATTR_POS = 0, ATTR_AMP = 1, ATTR_CNT = 2 } ;
  enum { UNIF_CLR = 0, UNIF_PEL_LEN = 1, UNIF_SQ_SD = 2, UNIF_PAL = 3, UNIF_THR = 4, UNIF_CNT = 5 } ;

  GLuint _vbo_ids[ATTR_CNT];
  GLuint _unif_locs[UNIF_CNT];
  GLuint _attr_locs[ATTR_CNT];

  // Палитра
  RadarPalette* _pal;
};

#endif // RADARENGINE_H
