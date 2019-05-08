class CfgPatches {
	class grad_mtg {
		name = "Gruppe Adler Map Tiles Generator";
		units[] = {};
		weapons[] = {};
		requiredVersion = 1.82;
		requiredAddons[] = {"intercept_core", "3den"};
		author = "Willard";
		authors[] = {"Willard"};
		url = "https://github.com/gruppe-adler";
		version = "1.0";
		versionStr = "1.0";
		versionAr[] = {1,0};
	};
};
class Intercept {
    class Willard {
        class grad_mtg {
            pluginName = "grad_mtg";
        };
    };
};

class ctrlMapMain;
class grad_mtg_ctrlMap_sat: ctrlMapMain
{
    widthRailWay = 4;
    colorForest[] = {0.624,0.78,0.388,0.5};
    colorForestBorder[]={0,0,0,0};
    colorRailWay[]={1,0,0,1};
    fontInfo = "RobotoCondensedLight";
    fontLabel = "RobotoCondensedLight";
    drawShaded=0;

    class Legend
    {
        x=safeZoneW;
        y=safeZoneH;
        w=0;
        h=0;
        color[]={0,0,0,0};
        colorBackground[]={0,0,0,0};
        font="RobotoCondensedLight";
        sizeEX="0";
    };
};

class grad_mtg_ctrlMap_topo: grad_mtg_ctrlMap_sat
{
    maxSatelliteAlpha = 0;
};