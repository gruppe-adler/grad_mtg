#include <intercept.hpp>

// Std
#include <chrono>
#include <thread>
#include <filesystem>

#include <iostream>
#include <fstream>
#include <sstream>

// WINAPI
#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <GdiplusPixelFormats.h>
#include <Gdiplusimaging.h>
#include <atlimage.h>

// Json
#include <nlohmann/json.hpp>

#define RESOLUTION 256
#define LAYER "grad_mtg_map_layer"

using namespace intercept;

namespace fs = std::filesystem;

namespace nl = nlohmann;

using SQFPar = game_value_parameter;

bool stop = false;

fs::path basePath;

int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

void intercept::register_interfaces() {}

void intercept::pre_init() {
    intercept::sqf::diag_log("The grad_mtg is running!");
    sqf::set_variable(sqf::mission_namespace(), "grad_mtg_isRunning", false);

    basePath = "grad_mtg";

    if (!fs::exists(basePath)) {
        fs::create_directories(basePath);
    }
}

/*
GetEncoderClsid
From: https://docs.microsoft.com/en-us/windows/desktop/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
*/
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

/*
    Takes a screenshot, crops it the grid size and save it as a png
*/
bool takeScreenShot(std::string file_name, types::vector2 tl, int w, int h) {

    // get the window/rectangle
    RECT rc;
    HWND hwnd = FindWindow(_T("Arma 3"), NULL);
    if (hwnd == NULL)
    {
        std::cout << "Couldn't find Arma Window!" << std::endl;
        return false;
    }
    GetClientRect(hwnd, &rc);

    // create print dst
    HDC hdcScreenPrintDst = GetDC(NULL);
    HDC hdcPrintDst = CreateCompatibleDC(hdcScreenPrintDst);
    HBITMAP hbmpPrint = CreateCompatibleBitmap(hdcScreenPrintDst,
        rc.right - rc.left, rc.bottom - rc.top);
    SelectObject(hdcPrintDst, hbmpPrint);

    // create crop dst
    HDC hdcScreenCropDst = GetDC(NULL);
    HDC hdcCropDst = CreateCompatibleDC(hdcScreenCropDst);
    HBITMAP hbmpCropDst = CreateCompatibleBitmap(hdcScreenCropDst, w, h);
    SelectObject(hdcCropDst, hbmpCropDst);

    // Print the entire screen
    PrintWindow(hwnd, hdcPrintDst, PW_CLIENTONLY);

    // Crop the image
    BitBlt(hdcCropDst, 0, 0, w, h, hdcPrintDst, (int)tl.x, (int)tl.y, SRCCOPY);

    // init gdi
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Load the WINAPI Bitmap into a Gdi+ Bitmap
    auto gdiBitmap = new Gdiplus::Bitmap(hbmpCropDst, NULL);

    // Create target Bitmap with 24bitdepth
    auto gdiBitmapDst = new Gdiplus::Bitmap(RESOLUTION, RESOLUTION, PixelFormat24bppRGB);

    // Draw/Copy it
    auto gdiGraphics = new Gdiplus::Graphics(gdiBitmapDst);
    gdiGraphics->DrawImage(gdiBitmap, Gdiplus::Rect(0, 0, RESOLUTION, RESOLUTION));

    // Get class id for the JPG encoder
    CLSID pngClsid;
    GetEncoderClsid(L"image/jpeg", &pngClsid);

    // convert the filename into wchar_t because of winapi
    std::wstring widestr = std::wstring(file_name.begin(), file_name.end());
    const wchar_t* w_file_name = widestr.c_str();
    gdiBitmapDst->Save(w_file_name, &pngClsid, NULL);

    // Cleanup
    DeleteDC(hdcPrintDst);
    DeleteDC(hdcCropDst);

    DeleteObject(hbmpPrint);
    DeleteObject(hbmpCropDst);

    ReleaseDC(NULL, hdcScreenPrintDst);
    ReleaseDC(NULL, hdcScreenCropDst);

    delete gdiBitmap;
    delete gdiBitmapDst;
    delete gdiGraphics;

    Gdiplus::GdiplusShutdown(gdiplusToken);

    return true;
}

/*
    Calculates zoom factor for a 100m tile to be as wide as the set resolution 
 */
float calcZoomFactor(types::control map, client::invoker_lock* thread_lock) {
    const auto COORDS_CENTER = types::vector2(50, 50);
    const auto COORDS_TL = types::vector2(0, 100);
    const auto COORDS_BR = types::vector2(100, 0);

    float zoomFactor = 1.0f;
    float widthInPixels = 0.0f;

    while (abs(widthInPixels - RESOLUTION) > 0.1)
    {
        sqf::ctrl_map_anim_add(map, 0.0f, zoomFactor, COORDS_CENTER);
        sqf::ctrl_map_anim_commit(map);

        // wait for map to actually commit because ctrlMapWorldToScreen 
        // won't work directly after mapCommit
        while(!sqf::ctrl_map_anim_done(map)) {
            thread_lock->unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            thread_lock->lock();
        }

        // calculate width of 100m grid with current zoom factor
        auto screenTL = sqf::ctrl_map_world_to_screen(map, COORDS_TL);
        auto screenBR = sqf::ctrl_map_world_to_screen(map, COORDS_BR);
        widthInPixels = (screenBR.x - screenTL.x) / sqf::pixel_w();

        zoomFactor = zoomFactor / (RESOLUTION / widthInPixels);
    }

    return zoomFactor;
}

void mapTileGenerator(int levelOfDetail, int type = 0, int resumeOnRow = 0) {

    auto mapType = "topo";
    if (type > 0) {
        mapType = "sat";
    }

    auto tileSize = (int)(100 * pow(2, 8 - levelOfDetail));

    auto worldName = sqf::world_name();
    std::transform(worldName.begin(), worldName.end(), worldName.begin(), ::tolower);
    auto folderBasePath = basePath / worldName / mapType / std::to_string(levelOfDetail);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        client::invoker_lock thread_lock;

        sqf::set_variable(sqf::mission_namespace(), "grad_mtg_isRunning", true);

        sqf::cut_rsc(LAYER, "RscTitleDisplayEmpty", "PLAIN", 1.0f, false);
        auto display = sqf::get_variable(sqf::ui_namespace(), "RscTitleDisplayEmpty");
        auto map = sqf::ctrl_create(display, std::string("grad_mtg_ctrlMap_").append(mapType), 1);

        sqf::ctrl_set_position(map, sqf::safe_zone_x(), sqf::safe_zone_y(), sqf::safe_zone_w(), sqf::safe_zone_h());
        sqf::ctrl_commit(map, 0);

        auto numTiles = (int)floor(sqf::world_size() / tileSize);
        float zoomFactor = calcZoomFactor(map, &thread_lock);
        int startRow = resumeOnRow == 0 ? numTiles : (numTiles - resumeOnRow);

        std::stringstream hintStream;
        hintStream << "Starting with LOD: " << levelOfDetail << " MapType: " << mapType;
        sqf::hint(hintStream.str());

        for (int yPos = startRow; yPos >= 0; yPos--) {
            for (int xPos = 0; xPos <= numTiles; xPos++) {
                // check if we need to stop
                if (stop) {
                    sqf::cut_fade_out(LAYER, 0);
                    sqf::set_variable(sqf::mission_namespace(), "grad_mtg_isRunning", false);
                    return;
                }

                auto pos = types::vector2(xPos * tileSize + tileSize / 2, yPos * tileSize + tileSize / 2);

                sqf::ctrl_map_anim_add(map, 0.0f, zoomFactor * tileSize / 100, pos);
                sqf::ctrl_map_anim_commit(map);

                // wait for map to actually commit because ctrlMapWorldToScreen 
                // won't work directly after mapCommit
                while(!sqf::ctrl_map_anim_done(map)) {
                    thread_lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    thread_lock.lock();
                }

                // find top left and bottom right corner on the screen in shitty arma units
                // shitty (arma) units == absolute position
                auto coordsTL = types::vector2(xPos * tileSize, yPos * tileSize + tileSize);
                auto coordsBR = types::vector2(xPos * tileSize + tileSize, yPos * tileSize);
                auto screenTL = sqf::ctrl_map_world_to_screen(map, coordsTL);
                auto screenBR = sqf::ctrl_map_world_to_screen(map, coordsBR);

                // shitty units origin (in pixels) + shitty units converted to pixels
                auto pixelTL = types::vector2(
                    abs(sqf::safe_zone_x()) / sqf::pixel_w() + screenTL.x / sqf::pixel_w(),
                    abs(sqf::safe_zone_y()) / sqf::pixel_h() + screenTL.y / sqf::pixel_h()
                );

                auto w = (int)floor((screenBR.x - screenTL.x) / sqf::pixel_w());
                auto h = (int)floor((screenBR.y - screenTL.y) / sqf::pixel_h());

                thread_lock.unlock();

                std::string filePath("");

                try {
                    // arma coords start in the bootom left but tiles should start in the top left corner
                    auto path = fs::absolute(folderBasePath / std::to_string(xPos));

                    if (!fs::exists(path)) {
                        fs::create_directories(path);
                    }

                    filePath = (path / std::to_string((int)numTiles - yPos)).string();
                    filePath.append(".png");
                }
                catch (fs::filesystem_error& ex) {
                    sqf::diag_log(ex.what());
                }

                if (filePath.empty()) {
                    sqf::diag_log("FilePath failed!");
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    takeScreenShot(filePath, pixelTL, w, h);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
                
                thread_lock.lock();
            }
        }

        sqf::cut_fade_out(LAYER, 0);
        sqf::set_variable(sqf::mission_namespace(), "grad_mtg_isRunning", false);
    }
}

game_value generateMetaFile(game_state &gs) {
    auto worldSize = (int)sqf::world_size();
    auto worldName = sqf::world_name();
    std::transform(worldName.begin(), worldName.end(), worldName.begin(), ::tolower);
    auto gridOffsetX = (int)sqf::get_number(sqf::config_entry(sqf::config_file()) >> ("CfgWorlds") >> worldName >> ("Grid") >> ("offsetX"));
    auto gridOffsetY = (int)sqf::get_number(sqf::config_entry(sqf::config_file()) >> ("CfgWorlds") >> worldName >> ("Grid") >> ("offsetY"));

    nl::json ret;
    ret["worldName"] = worldName;
    ret["worldSize"] = worldSize;
    ret["displayName"] = sqf::get_text(sqf::config_entry(sqf::config_file()) >> ("CfgWorlds") >> worldName >> ("description"));
    ret["minLod"] = 0;
    ret["maxLod"] = 8;
    ret["grid"] = { {"offsetX", gridOffsetX }, {"offsetY", gridOffsetY } };
    ret["layers"] = { { {"name", "Topographic"}, {"path", "topo/"} }, { {"name", "Satellite"}, {"path", "sat/"} } };

    auto locations = sqf::nearest_locations(types::vector3(worldSize / 2, worldSize / 2, 0), std::vector<std::string> {"nameVillage", "nameCity", "nameCityCapital"}, worldSize);

    for (auto& location : locations) {
        ret["locations"].push_back({ { "name", sqf::text(location) }, { "pos", std::vector<float>{sqf::position(location).x, sqf::position(location).y, sqf::position(location).z}} });
    }
    
    auto metaPath = basePath / worldName;

    if (!fs::exists(metaPath)) {
        fs::create_directories(metaPath);
    }

    std::ofstream out(metaPath / "meta.json");
    out << std::setw(4) << ret << std::endl;
    out.close();
    return true;
}

game_value startMapTileGen(game_state &gs, SQFPar right_arg) {

    int lod = -1;
    int type = 0;
    int row = 0;

    if(right_arg.type_enum() == game_data_type::ARRAY) {
        if (right_arg.size() < 2 || right_arg.size() > 3) {
            gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "Wrong size of argument array"sv);
            return false;
        }

        if (right_arg[0].type_enum() != game_data_type::SCALAR) {
            gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "LOD has to be a number"sv);
            return false;
        }

        if (right_arg[1].type_enum() != game_data_type::SCALAR) {
            gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "Map-Type has to be a number"sv);
            return false;
        }

        if (right_arg.size() == 3) {
            if (right_arg[2].type_enum() != game_data_type::SCALAR) {
                gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "Row has to be a number"sv);
                return false;
            }
            else {
                row = (int)right_arg[2];
            }
        }
        lod = (int)right_arg[0];
        type = (int)right_arg[1];

    } else { // SCALAR
        lod = (int)right_arg;
    }
    
    if (lod < 0 || lod > 8) {
        gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "LOD has to be >= 0 and <= 8"sv);
        return false;
    }

    stop = false;
    std::thread iteraterThread(mapTileGenerator, lod, type, row);
    iteraterThread.detach();
    return true;
}

void completeGen(int min, int max) {
    for (int i = min; i <= max; i++) {
        if(!stop) mapTileGenerator(i, 0);
        if(!stop) mapTileGenerator(i, 1);
    }
}


game_value startMapTileGenComplete(game_state& gs, SQFPar right_arg) {
    if (right_arg.size() != 2) {
        gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "Right parameter count != 2"sv);
        return false;
    }
    else if (right_arg[0].type_enum() != game_data_type::SCALAR || right_arg[1].type_enum() != game_data_type::SCALAR) {
        gs.set_script_error(types::game_state::game_evaluator::evaluator_error_type::assertion_failed, "NaN"sv);
        return false;
    }

    stop = false;

    std::thread iteraterThread(completeGen, (int)right_arg[0], (int)right_arg[1]);
    iteraterThread.detach();
    return true;
}

game_value stopMapTileGen() {
    stop = true;
    return true;
}


void intercept::pre_start() {
    static auto grad_mtg_start = client::host::register_sqf_command("gradMtgStart", "Starts the map tile generation", startMapTileGen, game_data_type::BOOL, game_data_type::SCALAR);
    static auto grad_mtg_start_arr = client::host::register_sqf_command("gradMtgStart", "Starts the map tile generation", startMapTileGen, game_data_type::BOOL, game_data_type::ARRAY);
    static auto grad_mtg_stop = client::host::register_sqf_command("gradMtgStop", "Stops the map tile generation", userFunctionWrapper<stopMapTileGen>, game_data_type::BOOL);
    static auto grad_mtg_meta = client::host::register_sqf_command("gradMtgMeta", "Generates a meta.json", generateMetaFile, game_data_type::BOOL);
    static auto grad_mtg_complete = client::host::register_sqf_command("gradMtgCompleteStart", "Starts the map tile generation for the whole map", startMapTileGenComplete, game_data_type::BOOL, game_data_type::ARRAY);
}