#ifndef WATERMARK_H
#define WATERMARK_H

#include <QMainWindow>

namespace Ui {
class WaterMark;
}

class WaterMark : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit WaterMark(QWidget *parent = 0);
    ~WaterMark();
    
private slots:
    void on_btnBrowseIn_clicked();
    void on_btnBrowseOut_clicked();
    void on_btnConvert_clicked();
    void on_btnBrowseInDir_clicked();
private:
    Ui::WaterMark *ui;
};

#endif // WATERMARK_H
