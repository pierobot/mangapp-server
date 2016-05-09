#include "image.hpp"
#include "utf8.hpp"

#include <iostream>
#include <memory>

namespace mangapp
{
    image::image(std::string const & contents) :
        m_mat(cv::imdecode(cv::InputArray(std::vector<uchar>(contents.cbegin(), contents.end())), 1))
    {
    }

    image::~image()
    {
    }

    void image::resize(int32_t width, int32_t height, bool cover_crop /* = true */)
    {
        if (m_mat.data == nullptr)
            return;

        cv::Size size(width, height);
        if (cover_crop == true)
        {
            cv::Size mat_size(m_mat.cols, m_mat.rows);
            // This code assumes the cover looks like the following
            //  -------------------------
            // |   front   | |   back    |
            // |           | |           |
            // |           | |           |
            // |           | |           |
            // |           | |           |
            // |           | |           |
            // |           | |           |
            //  -------------------------
            //  We want the front

            if (m_mat.cols > m_mat.rows)
            {
                // Using cv::Rect on the stack always results in bogus values, for whatever reason :/
                std::unique_ptr<cv::Rect> region_front(new cv::Rect(0, 0, get_width() / 2, get_height())); 
                cv::Mat cropped_mat;
                m_mat(*region_front).copyTo(cropped_mat);
                

                cv::Size new_size(region_front->width, region_front->height);
                m_mat = cropped_mat;
                resize(width, height, false);
                /*cv::resize(cropped_mat, m_mat, new_size, 0.0, 0.0, cv::INTER_AREA);*/
                
                return;
            }
        }

        if (m_mat.data != nullptr)
            cv::resize(m_mat, m_mat, size, 0.0, 0.0, cv::INTER_AREA);

    }

    int32_t const image::get_width() const
    {
        return m_mat.data != nullptr ? m_mat.cols : 0;
    }

    int32_t const image::get_height() const
    {
        return m_mat.data != nullptr ? m_mat.rows : 0;
    }

    void image::save(std::string const & path, std::string const & name) const
    {
        if (m_mat.data != nullptr)
            cv::imwrite(path + name, m_mat);
    }

    std::string const image::contents(std::string const & extension) const
    {
        std::string contents;

        if (m_mat.data != nullptr)
        {
            std::vector<uchar> buffer;

            cv::imencode(extension, m_mat, buffer);

            contents = std::string(reinterpret_cast<char*>(buffer.data()), buffer.size());
        }

        return contents;
    }
}