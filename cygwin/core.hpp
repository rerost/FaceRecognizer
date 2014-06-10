#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <map>

typedef std::pair<std::string, cv::Rect> pair_id_rect;

enum faceParts{ face, eye, nose, mouth };

struct CascadeData{
        CascadeData(const cv::Mat& image,
                        std::string name,
                        const cv::Size rectSize,
                        const int rectQuantity,
                        const cv::Scalar color,
                        int cnt,
                        faceParts id)
                :img(image.clone()),
                detector(name),
                minRectSize(rectSize),
                minRectQuantity(rectQuantity),
                rectColor(color),
                count(cnt),
                cascadeId(id)
        {}

        cv::Mat         img;
        std::string detector;
        cv::Size        minRectSize;      //scale
        int                     minRectQuantity;
        cv::Scalar      rectColor;
        int                     count;
        faceParts cascadeId;
};

template<typename T>
struct Point{
        T x;
        T y;
        Point() : x(-1),y(-1) {}
        Point(T x_, T y_) : x(x_), y(y_) {}
};

class PersonData{
        public:
                int id;
                cv::Rect eyeR;
                cv::Rect eyeL;
                cv::Rect nose;
                cv::Rect mouse;
                cv::Mat image;

                //未定義
                int face_color;
                int eyeMany;
                int noseMany;
                int mouseMany;

                PersonData(){
                        id = -1;
                }

                PersonData(int id_) : id(id_) {}
                PersonData(int id_, cv::Mat& image_) : id(id_), image(image.clone()){}
                //以下のコンストラクタは想定しない。
                //PersonData(int ID_, Point eyeR_, Point eyeL_,... )
};

