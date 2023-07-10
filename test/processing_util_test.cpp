#include <rad/processing_util.hpp>

#include "test_file_manager.hpp"

#include <catch2/catch_test_macros.hpp>

namespace fs = std::filesystem;

TEST_CASE("[processing_util] - get_file_paths_from_root", "[rad]")
{
    TestFileManager mgr{TestFileManager::Params{}};

    auto files = rad::get_file_paths_from_root(mgr.root().string());

    REQUIRE(files.size() == 10);
    for (int i{0}; auto file : files)
    {
        fs::path exp = mgr.root() / fmt::format("test_img_{}.jpg", i);
        REQUIRE(file == exp);
        ++i;
    }
}

TEST_CASE("[processing_util] - load_image", "[rad]")
{
    TestFileManager::Params params{.num_files = 1};
    const std::string exp_name = "test_img_0";

    SECTION("jpg")
    {
        params.ext = "jpg";
        TestFileManager mgr{params};

        fs::path p       = mgr.root() / fmt::format("{}.{}", exp_name, params.ext);
        auto [name, img] = rad::load_image(p.string());

        REQUIRE(name == exp_name);
        REQUIRE(img.size() == params.size);
        REQUIRE(img.type() == params.type);
    }

    SECTION("JPG")
    {
        params.ext = "JPG";
        TestFileManager mgr{params};

        fs::path p       = mgr.root() / fmt::format("{}.{}", exp_name, params.ext);
        auto [name, img] = rad::load_image(p.string());

        REQUIRE(name == exp_name);
        REQUIRE(img.size() == params.size);
        REQUIRE(img.type() == params.type);
    }

    SECTION("png")
    {
        params.ext = "png";
        TestFileManager mgr{params};

        fs::path p       = mgr.root() / fmt::format("{}.{}", exp_name, params.ext);
        auto [name, img] = rad::load_image(p.string());

        REQUIRE(name == exp_name);
        REQUIRE(img.size() == params.size);
        REQUIRE(img.type() == params.type);
    }
}

TEST_CASE("[processing_util] - create_result_dir", "[rad]")
{
    fs::path root = fs::absolute("./test_root");
    rad::create_result_dir(root.string(), "app");

    REQUIRE(fs::exists(root / "app"));
    fs::remove_all(root);
}

TEST_CASE("[processing_util] - save_result", "[rad]")
{
    const fs::path root        = fs::absolute("./test_root");
    const std::string name     = "test_app";
    const std::string img_name = "test_img.jpg";
    const auto img_path        = root / (name + "/" + img_name);
    const cv::Size size{64, 64};

    fs::create_directories(root / name);

    SECTION("Empty image")
    {
        rad::save_result({}, root.string(), name, img_name);
        REQUIRE_FALSE(fs::exists(img_path));
    }

    SECTION("Single channel image")
    {
        SECTION("uint image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_8UC1);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }

        SECTION("float image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_32F);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }
    }

    SECTION("RGB image")
    {
        SECTION("uint image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_8UC3);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }

        SECTION("float image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_32FC3);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }
    }

    SECTION("RGBA image")
    {
        SECTION("uint image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_8UC4);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }

        SECTION("float image")
        {
            cv::Mat img = cv::Mat::ones(size, CV_32FC4);
            rad::save_result(img, root.string(), name, img_name);
            REQUIRE(fs::exists(img_path));
        }
    }

    fs::remove_all(root);
}
