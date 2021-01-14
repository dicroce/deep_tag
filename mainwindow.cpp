
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createrectdialog.h"

#include <QFileDialog>
#include <QImage>
#include <QInputDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPainter>
#include <QMouseEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QUuid>
#include <QSound>
#include <QStandardPaths>
#include <QDialogButtonBox>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <memory>
#include <stdexcept>
#include <cmath>

// - Linux packaging
//   - make install
// - Add help
// - Make demo youtube video.
// - Update github page.
// - Do final 1.0 windows & linux builds
// - Add shift click keyboard shortcuts for fractional box movement and scaling.

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _current_clip()
    , _current_frame_index(-1)
    , _resize_timer()
    , _widgets()
    , _rects()
    , _nextRectKey(0)
    , _drag()
    , _startup_dialog(nullptr)
    , _tag_shortcut(nullptr)
    , _annotation_path()
    , _image_path()
    , _cfg_path()
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/images/dt.png"));

    // We only want to do a real resize after the user is done reconfiguring the
    // window so set a timer here.
    _resize_timer.setSingleShot(true);
    connect(&_resize_timer, SIGNAL(timeout()), SLOT(on_resize_done()));

    _widgets.image_label = _find_child<QLabel>("image_label");
    _widgets.timeline_slider = _find_child<QSlider>("timeline_slider");
    _widgets.class_list = _find_child<QListWidget>("class_list");
    _widgets.rects_list = _find_child<QListWidget>("rects_list");
    _widgets.video_container = _find_child<QWidget>("video_container");
    _widgets.dataset_path = _find_child<QLabel>("dataset_path");
    _widgets.num_annotations = _find_child<QLabel>("num_annotations");

    _widgets.image_label->setMouseTracking(true);
    _widgets.image_label->installEventFilter(this);

    connect(_widgets.rects_list, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(on_rect_list_clicked(QListWidgetItem*)));

    _startup_dialog = new StartupDialog(this);

    QTimer::singleShot(0, this, SLOT(show_startup_dialog()));

    _find_child<QWidget>("container")->setEnabled(false);

    auto tag_button = _find_child<QPushButton>("tag_button");

    connect(tag_button, SIGNAL(clicked()), this, SLOT(tag_pressed()));

    new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(tag_pressed()));
    new QShortcut(QKeySequence(Qt::Key_Right), this, SLOT(on_next_frame_clicked()));
    new QShortcut(QKeySequence(Qt::Key_Left), this, SLOT(on_prev_frame_clicked()));
    new QShortcut(QKeySequence(Qt::Key_BracketLeft), this, SLOT(shrink_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_BracketRight), this, SLOT(grow_selected_rect()));

    new QShortcut(QKeySequence(Qt::Key_W), this, SLOT(up_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_S), this, SLOT(down_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_A), this, SLOT(left_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_D), this, SLOT(right_selected_rect()));

    new QShortcut(QKeySequence(Qt::Key_Apostrophe), this, SLOT(grow_vertical_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_Semicolon), this, SLOT(shrink_vertical_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(grow_horizontal_selected_rect()));
    new QShortcut(QKeySequence(Qt::Key_Period), this, SLOT(shrink_horizontal_selected_rect()));

    new QShortcut(QKeySequence(Qt::Key_Tab), this, SLOT(next_selected()));
}

MainWindow::~MainWindow()
{
    delete _startup_dialog;
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    // In our window resize we continually reset a timer that fires in half a second...
    // This allows us to have an event that occurs half a second after the user is done
    // reconfiguring window.
    _resize_timer.start(500);
    QWidget::resizeEvent(e);
}

double distance(double x1, double y1, double x2, double y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

bool MainWindow::eventFilter(QObject*, QEvent* event)
{
    if(!_current_clip)
        return false;

    auto evt_type = event->type();

    if(evt_type == QEvent::MouseButtonPress ||
       evt_type == QEvent::MouseButtonRelease ||
       evt_type == QEvent::MouseMove)
    {
        auto me = dynamic_cast<QMouseEvent*>(event);

        if(me)
        {
            auto mouse_position = me->pos();

            int stage_width = _widgets.video_container->width();
            int stage_height = _widgets.video_container->height();

            auto x = (double)mouse_position.x() / (double)stage_width;
            auto y = (double)mouse_position.y() / (double)stage_height;

            if(evt_type == QEvent::MouseButtonPress)
            {
                if(!_drag.dragging)
                {
                    // First find which rect the click belongs to...

                    auto closest_rect = _rects.rects.begin();
                    double smallest_delta = 1.0;
                    for(auto i = closest_rect, e = _rects.rects.end(); i != e; ++i)
                    {
                        auto x1 = i->second.x;
                        auto y1 = i->second.y;
                        auto x2 = x1 + i->second.w;
                        auto y2 = y1 + i->second.h;

                        auto d_tl = distance(x, y, x1, y1);
                        auto d_tr = distance(x, y, x2, y1);
                        auto d_br = distance(x, y, x2, y2);
                        auto d_bl = distance(x, y, x1, y2);

                        if(d_tl < smallest_delta)
                        {
                            closest_rect = i;
                            smallest_delta = d_tl;
                        }
                        if(d_tr < smallest_delta)
                        {
                            closest_rect = i;
                            smallest_delta = d_tr;
                        }
                        if(d_br < smallest_delta)
                        {
                            closest_rect = i;
                            smallest_delta = d_br;
                        }
                        if(d_bl < smallest_delta)
                        {
                            closest_rect = i;
                            smallest_delta = d_bl;
                        }
                    }

                    auto dk = _is_drag_start(closest_rect->second, x, y);

                    if(dk != DK_NONE)
                    {
                        _drag.dragging = true;
                        _drag.rect_key = closest_rect->first;
                        _drag.kind = dk;
                        _drag.start_mouse_x = x;
                        _drag.start_mouse_y = y;
                        _drag.last_mouse_x = x;
                        _drag.last_mouse_y = y;

                        _rects.selected_key = closest_rect->first;

                        // Re-intialize the tracker....
                        if(!closest_rect->second.tracker.empty())
                        {
                            if(closest_rect->second.tracking == TA_KCF)
                                closest_rect->second.tracker = TrackerKCF::create();
                            else if(closest_rect->second.tracking == TA_MIL)
                                closest_rect->second.tracker = TrackerMIL::create();
                            else if(closest_rect->second.tracking == TA_CSRT)
                                closest_rect->second.tracker = TrackerCSRT::create();
                            closest_rect->second.tracker_initialized = false;
                        }

                        // Update the selected item in the rects ui list.
                        for(int i = 0; i < _widgets.rects_list->count(); ++i)
                        {
                            if(_widgets.rects_list->item(i)->text().toStdString() == _rects.selected_key)
                            {
                                _widgets.rects_list->setCurrentRow(i);
                                _update_status();
                            }
                        }
                    }
                }
            }
            else if(evt_type == QEvent::MouseButtonRelease)
            {
                if(_drag.dragging)
                    _render_frame();
                _drag.dragging = false;
                _drag.kind = DK_NONE;
            }
            else if(evt_type == QEvent::MouseMove)
            {
                if(_drag.dragging)
                {
                    _drag.last_mouse_x = x;
                    _drag.last_mouse_y = y;

                    auto r = _rects.rects[_drag.rect_key];
                    _drag_update_rect(r);
                    // publish our changes back into _rects.rects...
                    _rects.rects[_drag.rect_key] = r;

                    _render_frame();
                }
            }
        }

        return true;
    }

    return false;
}

void MainWindow::on_action_new_dataset_triggered()
{
    on_new_dataset_clicked();
}

void MainWindow::on_action_open_dataset_triggered()
{
    on_open_dataset_clicked();
}

void MainWindow::on_action_close_dataset_triggered()
{
    _clear_ui();

    _find_child<QWidget>("container")->setEnabled(false);
}

void MainWindow::on_action_exit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_action_tag_triggered()
{
    tag_pressed();
}

void MainWindow::on_new_dataset_clicked()
{

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Create Dataset Directory"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        _startup_dialog->close();

        _clear_ui();

        _find_child<QWidget>("container")->setEnabled(false);

        _annotation_path = (dir + QDir::separator() + "annotations").toStdString();
        _image_path = (dir + QDir::separator() + "images").toStdString();
        _cfg_path = dir.toStdString();

        _write_cfg(dir.toStdString());

        _find_child<QWidget>("container")->setEnabled(true);
    }
}

void MainWindow::on_open_dataset_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Dataset Directory"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        _startup_dialog->close();

        _clear_ui();

        _find_child<QWidget>("container")->setEnabled(false);

        _cfg_path = dir.toStdString();

        _read_cfg((dir + QDir::separator() + "deep_tag.cfg").toStdString());

        _widgets.dataset_path->setText(QString::fromStdString(_cfg_path));

        _find_child<QWidget>("container")->setEnabled(true);

        _update_ui();
    }
}

void MainWindow::on_open_video_button_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Open Video File"), QDir::homePath(), tr("MP4 (*.mp4);; AVI (*.avi)")
    );

    if(!fileName.isEmpty())
    {
        // Open our video file...
        _current_clip = make_unique<VideoCapture>(fileName.toStdString());
        _current_frame_index = 0;

        // Set our timeline slider maximum to the index of the last frame in the clip.
        _widgets.timeline_slider->setMaximum(_num_frames_in_clip(_current_clip) - 1);

        // Redraw
        _render_frame();
    }
}

void MainWindow::on_resize_done()
{
    // Our true handler for resizing.
    _render_frame();
}

void MainWindow::on_next_frame_clicked()
{
    // If we have a clip and adding 1 to our current position would not exceed the number of frame in the clip do it
    if(_current_clip && ((_current_frame_index+1) < (int)_num_frames_in_clip(_current_clip)))
    {
        ++_current_frame_index;
        _track_rects();

        // Update our slider
        _widgets.timeline_slider->setSliderPosition(_current_frame_index);
    }
}

void MainWindow::on_prev_frame_clicked()
{
    // If we have a clip and subtracting 1 from our current position would not go negative do it
    if(_current_clip && (_current_frame_index > 0))
    {
        --_current_frame_index;
        _render_frame();

        // Update our slider
        _widgets.timeline_slider->setSliderPosition(_current_frame_index);
    }
}

void MainWindow::on_timeline_slider_valueChanged(int value)
{
    if(_current_clip)
    {
        _current_frame_index = value;
        _render_frame();
    }
}

void MainWindow::on_create_class_button_clicked()
{
    bool ok = false;
    auto class_name_text = QInputDialog::getText(this, tr("Create Class"), tr("Class Name:"), QLineEdit::Normal, QDir::home().dirName(), &ok);

    if(ok)
    {
        _widgets.class_list->addItem(class_name_text);

        _write_cfg(_cfg_path);
    }

    _update_ui();
}

void MainWindow::on_destroy_class_button_clicked()
{
    for(auto item: _widgets.class_list->selectedItems())
    {
        delete _widgets.class_list->takeItem(_widgets.class_list->row(item));
    }

    _write_cfg(_cfg_path);

    _update_ui();
}

void MainWindow::on_create_rect_button_clicked()
{
    auto crd = new CreateRectDialog(this);

    if(crd)
    {
        crd->setModal(true);

        auto ccb = crd->findChild<QComboBox*>("class_combo_box");

        if(!ccb)
            throw std::runtime_error("Unable to find class_combo_box");

        for(auto i = 0; i < _widgets.class_list->count(); ++i)
            ccb->addItem(_widgets.class_list->item(i)->text());

        auto tcb = crd->findChild<QComboBox*>("tracking_combo_box");

        if(!tcb)
            throw std::runtime_error("Unable to find tracking_combo_box");

        tcb->addItem(QString::fromStdString(_tracking_algorithm_to_name(tracking_algorithm::TA_CSRT)));
        tcb->addItem(QString::fromStdString(_tracking_algorithm_to_name(tracking_algorithm::TA_MIL)));
        tcb->addItem(QString::fromStdString(_tracking_algorithm_to_name(tracking_algorithm::TA_KCF)));
        tcb->addItem(QString::fromStdString(_tracking_algorithm_to_name(tracking_algorithm::TA_NONE)));

        crd->exec();

        if(crd->accepted())
        {
            auto tar = crd->findChild<QCheckBox*>("tracking_auto_resize");

            if(!tar)
                throw std::runtime_error("Unable to find tracking_auto_resize");

            rect_info ri;

            ri.class_name = ccb->currentText().toStdString();
            ri.x = ri.y = 0.10;
            ri.w = ri.h = 0.25;
            ri.tracking = _name_to_tracking_algorithm(tcb->currentText().toStdString());
            ri.tracking_auto_resize = (tar->checkState() == Qt::Checked) ? true : false;

            if(ri.tracking == TA_KCF)
                ri.tracker = TrackerKCF::create();
            else if(ri.tracking == TA_MIL)
                ri.tracker = TrackerMIL::create();
            else if(ri.tracking == TA_CSRT)
                ri.tracker = TrackerCSRT::create();

            ri.tracker_initialized = false;

            // rects are inserted under incrementing numeric key...
            auto key = to_string(_nextRectKey);
            _rects.rects.insert(make_pair(key, ri));
            _rects.selected_key = key;
            ++_nextRectKey;
        }

        delete crd;
    }

    _update_ui();

    _render_frame();
}

void MainWindow::on_destroy_rect_button_clicked()
{
    auto selectedRects = _widgets.rects_list->selectedItems();

    if(selectedRects.count() != 1)
    {
        QMessageBox errMsg;
        errMsg.setWindowTitle("Error");
        errMsg.setText("Please select a rect before removing.");
        errMsg.exec();
        return;
    }
    else
    {
        _rects.rects.erase(selectedRects[0]->text().toStdString());
        _update_ui();
        _render_frame();
    }
}

void MainWindow::on_rect_list_clicked(QListWidgetItem*)
{
    auto selected = _widgets.rects_list->selectedItems();

    if(selected.count() != 1)
        throw std::runtime_error("Unable to select single rect list item.");

    _rects.selected_key = selected[0]->text().toStdString();

    _update_status();
    _render_frame();
}

void MainWindow::show_startup_dialog()
{
    if(_startup_dialog)
    {
        _startup_dialog->setModal(true);

        auto create_new_dataset = _startup_dialog->findChild<QPushButton*>("create_new_dataset");

        connect(create_new_dataset, SIGNAL(clicked()),
            this, SLOT(on_new_dataset_clicked()));

        auto open_dataset = _startup_dialog->findChild<QPushButton*>("open_dataset");

        connect(open_dataset, SIGNAL(clicked()),
            this, SLOT(on_open_dataset_clicked()));

        _startup_dialog->exec();
    }
}

void MainWindow::tag_pressed()
{
    QSound::play(":/sounds/shutter.wav");

    _write_annotation(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());

    on_next_frame_clicked();

    _update_ui();
}

void MainWindow::grow_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x -= 0.01;
        r.y -= 0.01;
        r.w += 0.02;
        r.h += 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::up_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.y -= 0.01;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::down_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.y += 0.01;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::left_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x -= 0.01;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::right_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x += 0.01;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::shrink_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x += 0.01;
        r.y += 0.01;
        r.w -= 0.02;
        r.h -= 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::shrink_vertical_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.y += 0.01;
        r.h -= 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::grow_vertical_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.y -= 0.01;
        r.h += 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::shrink_horizontal_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x += 0.01;
        r.w -= 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::grow_horizontal_selected_rect()
{
    if(!_rects.selected_key.empty())
    {
        auto r = _rects.rects[_rects.selected_key];
        r.x -= 0.01;
        r.w += 0.02;
        r.tracker_initialized = false;
        _rects.rects[_rects.selected_key] = r;

        _render_frame();
    }
}

void MainWindow::next_selected()
{
    if(!_rects.selected_key.empty())
    {
        auto found = _rects.rects.find(_rects.selected_key);

        ++found;
        if(found == _rects.rects.end())
            found = _rects.rects.begin();

        _rects.selected_key = found->first;

        _render_frame();
    }
}

void MainWindow::_update_ui()
{
    // Update the UI rects_list to match the rect_info list.
    _widgets.rects_list->clear();
    for(auto rp : _rects.rects)
    {
        _widgets.rects_list->addItem(QString::fromStdString(rp.first));
    }

    _widgets.num_annotations->setText(QString::fromStdString(to_string(_num_files_in_dir(_annotation_path))));
}

void MainWindow::_render_frame()
{
    // If we have a clip, then go ahead and render.
    if(_current_clip)
    {
        // Set our clip's frame position to the current value of _current_frame_index.
        _current_clip->set(cv::CAP_PROP_POS_FRAMES, _current_frame_index);

        // Get the current frame as an opencv Mat
        Mat frame;
        *_current_clip >> frame;

        // Convert our frame Mat into RGB
        cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        uint16_t w = 0, h = 0;
        _aspect_correct_dimensions(frame.cols, frame.rows, _widgets.image_label->width(), _widgets.image_label->height(), w, h);

        // Resize our frame into the current size of the QLabel we render into...
        cv::resize(frame, frame, cv::Size(w, h), 0, 0, cv::INTER_CUBIC);

        auto pixmap = QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, (int)frame.step, QImage::Format_RGB888));

        for(auto r: _rects.rects)
        {
            QPainter paint(&pixmap);

            QPen p;
            p.setColor(QColor(255, 255, 255, 255));
            if(r.first == _rects.selected_key)
                p.setColor(QColor(0, 0, 255, 255));
            p.setWidth(2);

            paint.setPen(p);

            auto pixmap_width = pixmap.width();
            auto pixmap_height = pixmap.height();

            paint.drawRect((int)(r.second.x * pixmap_width), (int)(r.second.y * pixmap_height), (int)(r.second.w * pixmap_width), (int)(r.second.h * pixmap_height));
        }

        // Draw...
        _widgets.image_label->setPixmap(pixmap);
    }
}

uint32_t MainWindow::_num_frames_in_clip(const std::unique_ptr<cv::VideoCapture>& clip)
{
    // Ask the passed clip for the number of frames it has...
    return (uint32_t)clip->get(cv::CAP_PROP_FRAME_COUNT);
}

void MainWindow::_aspect_correct_dimensions(uint16_t streamWidth, uint16_t streamHeight, uint16_t requestedWidth, uint16_t requestedHeight, uint16_t& destWidth, uint16_t& destHeight)
{
    destWidth = requestedWidth;
    destHeight = requestedHeight;

    // encode size
    if(streamWidth != 0 && streamHeight !=0)
    {
        uint16_t newEncodeWidth;
        uint16_t newEncodeHeight;

        if(requestedHeight != 0 && requestedWidth != 0)
        {
            float streamAspectRatio = streamWidth * 1.0f / streamHeight;
            float maxAspectRatio = requestedWidth * 1.0f / requestedHeight;
            float scaleFactor;

            if(maxAspectRatio < streamAspectRatio)
                scaleFactor = requestedWidth * 1.0f / streamWidth;
            else
                scaleFactor = requestedHeight * 1.0f / streamHeight;

            uint16_t scaledRoundedPixelWidth = (uint16_t)(streamWidth * scaleFactor + 0.5);
            uint16_t scaledRoundedPixelHeight = (uint16_t)(streamHeight * scaleFactor + 0.5);

            uint16_t multipleOfEightWidth = max( scaledRoundedPixelWidth / 8, 1) * 8;
            uint16_t multipleOfEightHeight = max( scaledRoundedPixelHeight / 8, 1) * 8;

            newEncodeWidth = multipleOfEightWidth;
            newEncodeHeight = multipleOfEightHeight;
        }
        else
        {
            newEncodeWidth = streamWidth;
            newEncodeHeight = streamHeight;
        }

        if(requestedWidth != newEncodeWidth)
            destWidth = newEncodeWidth;

        if(requestedHeight != newEncodeHeight)
            destHeight = newEncodeHeight;
    }
}

string MainWindow::_tracking_algorithm_to_name(tracking_algorithm algo) const
{
    if(algo == tracking_algorithm::TA_MIL)
        return "MIL";
    else if(algo == tracking_algorithm::TA_KCF)
        return "KCF";
    else if(algo == tracking_algorithm::TA_CSRT)
        return "CSRT";
    else return "NONE";
}

tracking_algorithm MainWindow::_name_to_tracking_algorithm(const string& name) const
{
    if(name == "NONE")
        return tracking_algorithm::TA_NONE;
    else if(name == "MIL")
        return tracking_algorithm::TA_MIL;
    else if(name == "KCF")
        return tracking_algorithm::TA_KCF;
    else return tracking_algorithm::TA_CSRT;
}

drag_kind MainWindow::_is_drag_start(const rect_info& ri, double x, double y)
{
    auto tl_d = distance(x, y, ri.x, ri.y);
    auto tr_d = distance(x, y, ri.x + ri.w, ri.y);
    auto br_d = distance(x, y, ri.x + ri.w, ri.y + ri.h);
    auto bl_d = distance(x, y, ri.x, ri.y + ri.h);

    auto threshold = 0.08;

    if((x >= ri.x) &&
       (y >= ri.y) &&
       (x < ri.x + ri.w) &&
       (y < ri.y + ri.h) &&
       QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier)
        return DK_CENTER;

    if(tl_d < tr_d && tl_d < br_d && tl_d < bl_d && tl_d < threshold)
        return DK_TL;

    if(tr_d < tl_d && tr_d < br_d && tr_d < bl_d && tr_d < threshold)
        return DK_TR;

    if(br_d < tl_d && br_d < tr_d && br_d < bl_d && br_d < threshold)
        return DK_BR;

    if(bl_d < tl_d && bl_d < tr_d && bl_d < br_d && bl_d < threshold)
        return DK_BL;

    return DK_NONE;
}

void MainWindow::_drag_update_rect(rect_info& r)
{
    auto br_x = r.x + r.w;
    auto br_y = r.y + r.h;

    if(_drag.kind == DK_TL)
    {
        r.x = _drag.last_mouse_x;
        r.y = _drag.last_mouse_y;
        r.w = br_x - r.x;
        r.h = br_y - r.y;
    }
    else if(_drag.kind == DK_TR)
    {
        r.w = _drag.last_mouse_x - r.x;
        r.y = _drag.last_mouse_y;
        r.h = br_y - r.y;
    }
    else if(_drag.kind == DK_BL)
    {
        r.x = _drag.last_mouse_x;
        r.w = br_x - r.x;
        r.h = _drag.last_mouse_y - r.y;
    }
    else if(_drag.kind == DK_BR)
    {
        r.w = _drag.last_mouse_x - r.x;
        r.h = _drag.last_mouse_y - r.y;
    }
    else if(_drag.kind == DK_CENTER)
    {
        r.x = _drag.last_mouse_x - (r.w / 2);
        r.y = _drag.last_mouse_y - (r.h / 2);
    }
}

void MainWindow::_update_status()
{
    auto current_rect = _rects.rects[_rects.selected_key];
    auto status = QString("%1 - (X=%2, Y=%3, W=%4, H=%5").arg(
        QString::fromStdString(current_rect.class_name),
        QString::number(current_rect.x, 'f', 2),
        QString::number(current_rect.y, 'f', 2),
        QString::number(current_rect.w, 'f', 2),
        QString::number(current_rect.h, 'f', 2));

    statusBar()->showMessage(status);
}

void MainWindow::_track_rects()
{
    // If we have a clip, then go ahead and render.
    if(_current_clip)
    {
        // Set our clip's frame position to the current value of _current_frame_index.
        _current_clip->set(cv::CAP_PROP_POS_FRAMES, _current_frame_index);

        // Get the current frame as an opencv Mat
        Mat frame;
        *_current_clip >> frame;

        // Convert our frame Mat into RGB
        cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        uint16_t w = 0, h = 0;
        _aspect_correct_dimensions(frame.cols, frame.rows, _widgets.image_label->width(), _widgets.image_label->height(), w, h);

        // Resize our frame into the current size of the QLabel we render into...
        cv::resize(frame, frame, cv::Size(w, h), 0, 0, cv::INTER_CUBIC);

        auto pixmap = QPixmap::fromImage(QImage(frame.data, frame.cols, frame.rows, (int)frame.step, QImage::Format_RGB888));

        for(int i = 0; i < _widgets.rects_list->count(); ++i)
        {
            auto key = _widgets.rects_list->item(i)->text().toStdString();
            auto r = _rects.rects[key];

            auto roi = Rect(r.x * w, r.y * h, r.w * w, r.h * h);

            if(!r.tracker.empty())
            {
                if(!r.tracker_initialized)
                {
                    r.tracker->init(frame, roi);
                    r.tracker_initialized = true;
                }
                else r.tracker->update(frame, roi);

                if(r.tracking_auto_resize)
                {
                    r.x = (double)roi.x / (double)w;
                    r.y = (double)roi.y / (double)h;
                    r.w = (double)roi.width / (double)w;
                    r.h = (double)roi.height / (double)h;
                }
                else
                {
                    // Compute the midpoint of the tracker box
                    auto roi_x = roi.x + (roi.width / 2);
                    auto roi_y = roi.y + (roi.height / 2);

                    // Position the user drawn box around the midpoint of the tracker box
                    r.x = (roi_x / (double)w) - (r.w / 2);
                    r.y = (roi_y / (double)h) - (r.h / 2);
                }
            }

            _rects.rects[key] = r;
        }
    }
}

void MainWindow::_read_cfg(const std::string& file_name)
{
    if(!QFile(QString::fromStdString(file_name)).exists())
        throw runtime_error("Unable to open dataset because configuration file is missing.");

    QFile file;
    file.setFileName(QString::fromStdString(file_name));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject obj = doc.object();

    QVariantMap main_map = obj.toVariantMap();

    auto annotation_path = main_map["annotation_path"].toString();
    if(!QDir(annotation_path).exists())
        QDir::root().mkpath(annotation_path);
    _annotation_path = annotation_path.toStdString();

    auto image_path = main_map["image_path"].toString();
    if(!QDir(image_path).exists())
        QDir::root().mkpath(image_path);
    _image_path = image_path.toStdString();

    _widgets.class_list->clear();
    auto value = obj.value("classes");
    auto class_array = value.toArray();
    foreach (const QJsonValue & v, class_array)
        _widgets.class_list->addItem(v.toString());
}

void MainWindow::_write_cfg(const std::string& ds_path)
{
    auto top_level_dir = QString::fromStdString(ds_path);
    if(!QDir(top_level_dir).exists())
        QDir::root().mkpath(top_level_dir);

    auto images_dir = top_level_dir + "/" + "images";
    if(!QDir(images_dir).exists())
        QDir::root().mkpath(images_dir);

    auto annotations_dir = top_level_dir + "/" + "annotations";
    if(!QDir(annotations_dir).exists())
        QDir::root().mkpath(annotations_dir);

    QString cfg_content = "{ \"annotation_path\": \"" + annotations_dir + "\", \"image_path\": \"" + images_dir + "\", \"classes\": [";

    auto num_classes = _widgets.class_list->count();
    for(int i = 0; i < num_classes; ++i)
    {
        cfg_content += "\"" + _widgets.class_list->item(i)->text() + "\"";
        if(i + 1 < num_classes)
            cfg_content += ", ";
    }
    cfg_content += "] }";


    QFile file(top_level_dir + QDir::separator() + "deep_tag.cfg");

    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << cfg_content << Qt::endl;
    }
}

void MainWindow::_clear_ui()
{
    _widgets.class_list->clear();
    _rects.rects.clear();
    _rects.selected_key.clear();

    _widgets.rects_list->clear();
    _widgets.dataset_path->clear();
    _widgets.num_annotations->clear();
}

int MainWindow::_num_files_in_dir(const std::string& path)
{
    QDir d(QString::fromStdString(path));
    d.setFilter(QDir::Files | QDir::NoSymLinks);
    return d.entryInfoList().size();
}

void MainWindow::_write_annotation(const std::string& base_name)
{
    if(_current_clip)
    {
        // Set our clip's frame position to the current value of _current_frame_index.
        _current_clip->set(cv::CAP_PROP_POS_FRAMES, _current_frame_index);

        // Get the current frame as an opencv Mat
        Mat frame;
        *_current_clip >> frame;

        int width = frame.cols;
        int height = frame.rows;

        // write our image...
        cv::imwrite(_image_path + QString(QDir::separator()).toStdString() + base_name + ".jpg", frame);

        std::string objects;
        for(auto i = _rects.rects.begin(), e = _rects.rects.end(); i!=e; ++i)
        {
            auto xmin = (int)(i->second.x * (double)width);
            auto ymin = (int)(i->second.y * (double)height);
            auto xmax = (int)((i->second.x + i->second.w) * (double)width);
            auto ymax = (int)((i->second.y + i->second.h) * (double)height);

            objects += QString("<object>\n"
                                   "    <name>%1</name>\n"
                                   "    <pose>Unspecified</pose>\n"
                                   "    <truncated>0</truncated>\n"
                                   "    <bndbox>\n"
                                   "        <xmin>%2</xmin>\n"
                                   "        <ymin>%3</ymin>\n"
                                   "        <xmax>%4</xmax>\n"
                                   "        <ymax>%5</ymax>\n"
                                   "    </bndbox>\n"
                                   "</object>\n").arg(QString::fromStdString(i->second.class_name),
                                                      QString::number(xmin),
                                                      QString::number(ymin),
                                                      QString::number(xmax),
                                                      QString::number(ymax)).toStdString();
        }

        auto annotation = QString("<annotation>\n"
                            "    <folder></folder>\n"
                            "    <filename>%1.jpg</filename>\n"
                            "    <source>\n"
                            "        <database></database>\n"
                            "        <annotation></annotation>\n"
                            "        <image></image>\n"
                            "        <flickrid></flickrid>\n"
                            "    </source>\n"
                            "    <owner>\n"
                            "    </owner>\n"
                            "    <size>\n"
                            "        <width>%2</width>\n"
                            "        <height>%3</height>\n"
                            "    </size>\n"
                            "    <segmented>0</segmented>\n"
                            "%4"
                            "</annotation>\n").arg(QString::fromStdString(base_name),
                                                   QString::number(width),
                                                   QString::number(height),
                                                   QString::fromStdString(objects));

        QFile annotation_file(QString::fromStdString(_annotation_path + QString(QDir::separator()).toStdString() + base_name + ".xml"));

        if(annotation_file.open(QIODevice::WriteOnly))
        {
            QTextStream out(&annotation_file);
            out << annotation;
            annotation_file.close();
        }
    }
}
