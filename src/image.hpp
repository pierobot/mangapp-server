#ifndef MANGAPP_IMAGE_HPP
#define MANGAPP_IMAGE_HPP

#include <cstdint>
#include <string>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace mangapp
{
    class image
    {
    public:
        image(std::string const & contents);
        virtual ~image();

        void resize(int32_t width, int32_t height, bool cover_crop = true);
        
        int32_t const get_width() const;
        int32_t const get_height() const;

        void save(std::string const & path, std::string const & name) const;
        std::string const contents(std::string const & extension) const;
    protected:
    private:
        cv::Mat m_mat;
    };
}

#endif