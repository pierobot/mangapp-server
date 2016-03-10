#include "image.hpp"
#include "utf8.hpp"

namespace mangapp
{
    image::image(std::string const & contents) :
        m_mat(cv::imdecode(cv::InputArray(std::vector<uchar>(contents.cbegin(), contents.end())), 1))
    {
    }

    image::~image()
    {
    }

    void image::resize(int32_t width, int32_t height)
    {
        cv::Size size(width, height);
        // overwrite the mat - shouldn't need to create a copy...
        if (m_mat.data != nullptr)
            cv::resize(m_mat, m_mat, size, 0.0, 0.0, cv::INTER_AREA);
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