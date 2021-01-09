#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "startupdialog.h"

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSlider>
#include <QListWidget>
#include <QShortcut>

#include <opencv2/core/utility.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/tracking.hpp>

#include <memory>
#include <map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct ui_widgets
{
    QLabel* image_label = {nullptr};
    QSlider* timeline_slider = {nullptr};
    QListWidget* class_list = {nullptr};
    QListWidget* rects_list = {nullptr};
    QWidget* video_container = {nullptr};
    QLabel* dataset_path = {nullptr};
    QLabel* num_annotations = {nullptr};
};

enum tracking_algorithm
{
    TA_NONE,
    TA_MIL,
    TA_KCF,
    TA_CSRT
};

struct rect_info
{
    std::string class_name;
    double x, y, w, h;
    tracking_algorithm tracking;
    bool tracking_auto_resize;
    cv::Ptr<cv::Tracker> tracker;
    bool tracker_initialized;
};

enum drag_kind
{
    DK_NONE,
    DK_TL,
    DK_TR,
    DK_BR,
    DK_BL,
    DK_CENTER
};

struct drag_state
{
    bool dragging = {false};
    std::string rect_key = {};
    drag_kind kind = {drag_kind::DK_NONE};
    double last_mouse_x = {0.0};
    double last_mouse_y = {0.0};
    double start_mouse_x = {0.0};
    double start_mouse_y = {0.0};
};

struct rects
{
    std::map<std::string, rect_info> rects;
    std::string selected_key;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* object, QEvent* event);

private slots:
    void on_action_new_dataset_triggered();
    void on_action_open_dataset_triggered();
    void on_action_close_dataset_triggered();
    void on_action_exit_triggered();
    void on_action_tag_triggered();

    void on_new_dataset_clicked();
    void on_open_dataset_clicked();

    void on_open_video_button_clicked();

    void on_resize_done();

    void on_next_frame_clicked();

    void on_prev_frame_clicked();

    void on_timeline_slider_valueChanged(int value);

    void on_create_class_button_clicked();

    void on_destroy_class_button_clicked();

    void on_create_rect_button_clicked();

    void on_destroy_rect_button_clicked();

    void on_rect_list_clicked(QListWidgetItem* item);

    void show_startup_dialog();

    void tag_pressed();

    void shrink_selected_rect();
    void grow_selected_rect();

    void up_selected_rect();
    void down_selected_rect();
    void left_selected_rect();
    void right_selected_rect();

private:
    void _update_ui();
    void _render_frame();
    uint32_t _num_frames_in_clip(const std::unique_ptr<cv::VideoCapture>& clip);
    void _aspect_correct_dimensions(uint16_t streamWidth, uint16_t streamHeight, uint16_t requestedWidth, uint16_t requestedHeight, uint16_t& destWidth, uint16_t& destHeight);

    std::string _tracking_algorithm_to_name(tracking_algorithm algo) const;
    tracking_algorithm _name_to_tracking_algorithm(const std::string& name) const;

    drag_kind _is_drag_start(const rect_info& ri, double x, double y, int stage_width, int stage_height);
    void _drag_update_rect(rect_info& r);

    void _update_status();

    void _track_rects();

    void _read_cfg(const std::string& file_name);
    void _write_cfg(const std::string& file_name);

    void _clear_ui();

    int _num_files_in_dir(const std::string& path);

    void _write_annotation(const std::string& base_name);

    template<typename WT>
    WT* _find_child(const std::string& name)
    {
        auto found_list = findChildren<WT*>(QString::fromStdString(name));

        if(found_list.size() == 0)
            throw std::runtime_error("Unable to find class: " + name);

        return found_list[0];
    }

    Ui::MainWindow *ui;
    std::unique_ptr<cv::VideoCapture> _current_clip;
    int _current_frame_index;
    QTimer _resize_timer;
    ui_widgets _widgets;
    rects _rects;
    int _nextRectKey;
    drag_state _drag;
    StartupDialog* _startup_dialog;
    QShortcut* _tag_shortcut;

    std::string _annotation_path;
    std::string _image_path;
    std::string _cfg_path;
};
#endif // MAINWINDOW_H
