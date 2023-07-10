#include <rad/onnx/session.hpp>

#include "model_file_manager.hpp"
#include <rad/onnx/env.hpp>

#include <algorithm>

#include <catch2/catch_test_macros.hpp>

namespace onnx = rad::onnx;

bool provider_present(std::vector<onnx::ExecutionProviders> const& providers,
                      onnx::ExecutionProviders provider)
{
    return std::find(providers.begin(), providers.end(), provider) != providers.end();
}

TEST_CASE("[session] - get_execution_providers", "[rad::onnx]")
{
    onnx::init_ort_api();

    auto providers = onnx::get_execution_providers();
    REQUIRE_FALSE(providers.empty());

    // CPU must always be present.
    REQUIRE(provider_present(providers, onnx::ExecutionProviders::cpu));

#if defined(RAD_ONNX_DML_ENABLED)
    REQUIRE(provider_present(providers, onnx::ExecutionProviders::dml));
#endif
}

TEST_CASE("[session] - is_provider_enabled", "[rad::onnx]")
{
    STATIC_REQUIRE(is_provider_enabled(rad::onnx::ExecutionProviders::cpu));

#if defined(RAD_ONNX_DML_ENABLED)
    STATIC_REQUIRE(is_provider_enabled(rad::onnx::ExecutionProviders::dml));
#endif
}

TEST_CASE("[session] - make_session_from_file", "[rad::onnx]")
{
    onnx::init_ort_api();
    ModelFileManager mgr;
    auto env = onnx::create_environment("session_test");

    SECTION("Invalid filename")
    {
        REQUIRE_THROWS(
            onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
                return std::wstring{};
            }));
    }

    SECTION("CPU session")
    {
        auto session =
            onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
                return mgr.get_model(ModelFileManager::Type::static_axes);
            });
        REQUIRE(static_cast<OrtSession*>(session) != nullptr);
    }

#if defined(RAD_ONNX_DML_ENABLED)
    SECTION("DML session")
    {
        auto opt = onnx::get_default_dml_session_options();
        auto session =
            onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
                return mgr.get_model(ModelFileManager::Type::static_axes);
            });
        REQUIRE(static_cast<OrtSession*>(session) != nullptr);
    }
#endif
}

TEST_CASE("[session] - get_input_shapes", "[rad::onnx]")
{
    onnx::init_ort_api();
    ModelFileManager mgr;
    auto env = onnx::create_environment("session_test");

    SECTION("Static axes")
    {
        auto session =
            onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
                return mgr.get_model(ModelFileManager::Type::static_axes);
            });
        REQUIRE(static_cast<OrtSession*>(session) != nullptr);

        auto shapes = onnx::get_input_shapes(session);
        REQUIRE(shapes.size() == 1);

        auto input_shape = shapes.front();
        REQUIRE(input_shape.size() == 4);
        REQUIRE(input_shape[0] == 1);
        REQUIRE(input_shape[1] == 1);
        REQUIRE(input_shape[2] == 32);
        REQUIRE(input_shape[3] == 32);
    }

    SECTION("Dynamic axes")
    {
        auto session =
            onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
                return mgr.get_model(ModelFileManager::Type::dynamic_axes);
            });
        REQUIRE(static_cast<OrtSession*>(session) != nullptr);

        auto shapes = onnx::get_input_shapes(session);
        REQUIRE(shapes.size() == 1);

        auto input_shape = shapes.front();
        REQUIRE(input_shape.size() == 4);
        REQUIRE(input_shape[0] == 1);
        REQUIRE(input_shape[1] == 1);
        REQUIRE(input_shape[2] == -1);
        REQUIRE(input_shape[3] == -1);
    }
}

TEST_CASE("[session] - get_output_shapes", "[rad::onnx]")
{
    onnx::init_ort_api();
    ModelFileManager mgr;
    auto env = onnx::create_environment("session_test");

    auto session = onnx::make_session_from_file(mgr.get_root(), env, [mgr](std::string) {
        return mgr.get_model(ModelFileManager::Type::static_axes);
    });
    REQUIRE(static_cast<OrtSession*>(session) != nullptr);

    auto shapes = onnx::get_output_shapes(session);
    REQUIRE(shapes.size() == 1);

    auto output_shape = shapes.front();
    REQUIRE(output_shape.size() == 2);
    REQUIRE(output_shape[0] == 1);
    REQUIRE(output_shape[1] == 10);
}
