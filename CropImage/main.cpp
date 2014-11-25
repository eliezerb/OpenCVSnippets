#include <iostream>
#include <opencv.hpp>

#define PI 3.14159265359

using namespace std;

double degree_to_radian(double angle)
{
    return angle * PI / 180;
}

cv::Mat rotate_image (cv::Mat image, double angle)
{
    // Rotates an OpenCV 2 image about it's centre by the given angle
    // (in radians). The returned image will be large enough to hold the entire
    // new image, with a black background

    cv::Size image_size = cv::Size(image.rows, image.cols);
    cv::Point image_center = cv::Point(image_size.height/2, image_size.width/2);

    // Convert the OpenCV 3x2 matrix to 3x3
    cv::Mat rot_mat = cv::getRotationMatrix2D(image_center, angle, 1.0);
    double row[3] = {0.0, 0.0, 1.0};
    cv::Mat new_row = cv::Mat(1, 3, rot_mat.type(), row);
    rot_mat.push_back(new_row);


    double slice_mat[2][2] = {
        {rot_mat.col(0).at<double>(0), rot_mat.col(1).at<double>(0)},
        {rot_mat.col(0).at<double>(1), rot_mat.col(1).at<double>(1)}
    };

    cv::Mat rot_mat_nontranslate = cv::Mat(2, 2, rot_mat.type(), slice_mat);

    double image_w2 = image_size.width * 0.5;
    double image_h2 = image_size.height * 0.5;

    // Obtain the rotated coordinates of the image corners
    std::vector<cv::Mat> rotated_coords;

    double image_dim_d_1[2] = { -image_h2, image_w2 };
    cv::Mat image_dim = cv::Mat(1, 2, rot_mat.type(), image_dim_d_1);
    rotated_coords.push_back(cv::Mat(image_dim * rot_mat_nontranslate));


    double image_dim_d_2[2] = { image_h2, image_w2 };
    image_dim = cv::Mat(1, 2, rot_mat.type(), image_dim_d_2);
    rotated_coords.push_back(cv::Mat(image_dim * rot_mat_nontranslate));


    double image_dim_d_3[2] = { -image_h2, -image_w2 };
    image_dim = cv::Mat(1, 2, rot_mat.type(), image_dim_d_3);
    rotated_coords.push_back(cv::Mat(image_dim * rot_mat_nontranslate));


    double image_dim_d_4[2] = { image_h2, -image_w2 };
    image_dim = cv::Mat(1, 2, rot_mat.type(), image_dim_d_4);
    rotated_coords.push_back(cv::Mat(image_dim * rot_mat_nontranslate));


    // Find the size of the new image
    vector<double> x_coords, x_pos, x_neg;
    for (int i = 0; i < rotated_coords.size(); i++)
    {
        double pt = rotated_coords[i].col(0).at<double>(0);
        x_coords.push_back(pt);
        if (pt > 0)
            x_pos.push_back(pt);
        else
            x_neg.push_back(pt);
    }

    vector<double> y_coords, y_pos, y_neg;
    for (int i = 0; i < rotated_coords.size(); i++)
    {
        double pt = rotated_coords[i].col(1).at<double>(0);
        y_coords.push_back(pt);
        if (pt > 0)
            y_pos.push_back(pt);
        else
            y_neg.push_back(pt);
    }


    double right_bound = *max_element(x_pos.begin(), x_pos.end());
    double left_bound = *min_element(x_neg.begin(), x_neg.end());
    double top_bound = *max_element(y_pos.begin(), y_pos.end());
    double bottom_bound = *min_element(y_neg.begin(), y_neg.end());

    int new_w = int(abs(right_bound - left_bound));
    int new_h = int(abs(top_bound - bottom_bound));

    // We require a translation matrix to keep the image centred
    double trans_mat[3][3] = {
        {1, 0, int(new_w * 0.5 - image_w2)},
        {0, 1, int(new_h * 0.5 - image_h2)},
        {0, 0, 1},
    };


    // Compute the transform for the combined rotation and translation
    cv::Mat aux_affine_mat = (cv::Mat(3, 3, rot_mat.type(), trans_mat) * rot_mat);
    cv::Mat affine_mat = cv::Mat(2, 3, rot_mat.type(), NULL);
    affine_mat.push_back(aux_affine_mat.row(0));
    affine_mat.push_back(aux_affine_mat.row(1));

    // Apply the transform
    cv::Mat output;
    cv::warpAffine(image, output, affine_mat, cv::Size(new_h, new_w), cv::INTER_LINEAR);

    return output;
}

cv::Size largest_rotated_rect(int h, int w, double angle)
{
    // Given a rectangle of size wxh that has been rotated by 'angle' (in
    // radians), computes the width and height of the largest possible
    // axis-aligned rectangle within the rotated rectangle.

    // Original JS code by 'Andri' and Magnus Hoff from Stack Overflow

    // Converted to Python by Aaron Snoswell (https://stackoverflow.com/questions/16702966/rotate-image-and-crop-out-black-borders)
    // Converted to C++ by Eliezer Bernart

    int quadrant = int(floor(angle/(PI/2))) & 3;
    double sign_alpha = ((quadrant & 1) == 0) ? angle : PI - angle;
    double alpha = fmod((fmod(sign_alpha, PI) + PI), PI);

    double bb_w = w * cos(alpha) + h * sin(alpha);
    double bb_h = w * sin(alpha) + h * cos(alpha);

    double gamma = w < h ? atan2(bb_w, bb_w) : atan2(bb_h, bb_h);

    double delta = PI - alpha - gamma;

    int length = w < h ? h : w;

    double d = length * cos(alpha);
    double a = d * sin(alpha) / sin(delta);
    double y = a * cos(gamma);
    double x = y * tan(gamma);

    return cv::Size(bb_w - 2 * x, bb_h - 2 * y);
}

cv::Mat crop_around_center(cv::Mat image, int height, int width)
{
    // Given a OpenCV 2 image, crops it to the given width and height,
    // around it's centre point

    cv::Size image_size = cv::Size(image.rows, image.cols);
    cv::Point image_center = cv::Point(int(image_size.height * 0.5), int(image_size.width * 0.5));

    if (width > image_size.width)
        width = image_size.width;

    if (height > image_size.height)
        height = image_size.height;

    int x1 = int(image_center.x - width  * 0.5);
    int x2 = int(image_center.x + width  * 0.5);
    int y1 = int(image_center.y - height * 0.5);
    int y2 = int(image_center.y + height * 0.5);


    return image(cv::Rect(cv::Point(y1, x1), cv::Point(y2,x2)));
}

void demo(cv::Mat image)
{
    // Demos the largest_rotated_rect function
    int image_height = image.rows;
    int image_width = image.cols;

    for (float i = 0.0; i < 360.0; i+=0.5)
    {
        cv::Mat image_orig = image.clone();
        cv::Mat image_rotated = rotate_image(image, i);

        cv::Size largest_rect = largest_rotated_rect(image_height, image_width, degree_to_radian(i));

        cv::Mat image_rotated_cropped = crop_around_center(
                    image_rotated,
                    largest_rect.height,
                    largest_rect.width
                    );

        cv::imshow("Original Image", image_orig);
        cv::imshow("Rotated Image", image_rotated);
        cv::imshow("Cropped image", image_rotated_cropped);

        if (char(cv::waitKey(15)) == 'q')
            break;
    }

}

int main (int argc, char* argv[])
{
    cv::Mat image = cv::imread(argv[1]);

    if (image.empty())
    {
        cout << "> The input image was not found." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Press [s] to begin or restart the demo" << endl;
    cout << "Press [q] to quit" << endl;

    while (true)
    {
        cv::imshow("Original Image", image);
        char opt = char(cv::waitKey(0));
        switch (opt) {
        case 's':
            demo(image);
            break;
        case 'q':
            return EXIT_SUCCESS;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}
