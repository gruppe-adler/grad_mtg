# Gruppe Adler Map Tile Generator

[![Build status](https://ci.appveyor.com/api/projects/status/fblksnwl7eb2stxy/branch/master?svg=true)](https://ci.appveyor.com/project/TheWillard/grad-mtg/branch/master)

---

Gruppe Adler Map Tile Generator (grad_mtg) is an Arma 3 Modification built on [intercept](https://github.com/intercept/intercept), which allows the generation of so called map tiles. 
  
Check out our [FAQ](#FAQ) at the bottom of this README for more info.

## Installation
Download the latest version of the mod from our [releases page](https://github.com/gruppe-adler/grad_mtg/releases). The only dependency, **which has to be loaded as well** is [intercept](https://steamcommunity.com/sharedfiles/filedetails/?id=1645973522). 

## How to use?
1. Start arma with grad_mtg and intercept.
2. Go into the editor and open the map you want to generate tiles of.
3.  Place a player-unit and start the scenario in singleplayer.
4. Once the mission has loaded press ESC and take a look at the debug console.
5. Type the command you wish (see below) into the `Execute` field and press `LOCAL EXEC`.
6. The statement should return `true` (check the black line directly below the text field) and there mustn't be any script errors.
7. **Close the ESC Menu** and wait five seconds to let grad_mtg do its thing.

## Commands
### `gradMtgStart`
|**Syntax**| |  
|---|---|  
|Description| Starts the generation of topographic map tiles for a specific LOD.|
|||
|Syntax| **gradMtgStart** levelOfDetail
|||
|Parameters|levelOfDetail `<NUMBER>`: LOD which should be generated|
|||
|Return Value| Generation started correctly? `<BOOLEAN>`|
|||
|Examples|`gradMtgStart 7;`|  
||`gradMtgStart 3;`|  
  
|**Alternative Syntax**| |  
|---|---|  
|Description| Starts the generation of map tiles for a specific LOD and map-type.|
|||
|Syntax| **gradMtgStart** [levelOfDetail, mapType]
|||
|Parameters|[levelOfDetail, mapType, startRow] `<ARRAY>`|
||levelOfDetail `<NUMBER>`: LOD which should be generated|
||mapType `<NUMBER>`: Map-Type (0 = topographic / 1 = satellite)|
||startRow *(optional)* `<NUMBER>`: Row with which should be stated. (The top most row is 0). All previous rows won't be exported. This allows to "continue" a previously aborted export. |
|||
|Return Value| Generation started correctly? `<BOOLEAN>`|
|||
|Examples|`gradMtgStart [3, 0];` Will generate topographic map-tiles for LOD 3|  
||`gradMtgStart [7, 1];` Will generate satellite map-tiles for LOD 7|
||`gradMtgStart [7, 1, 10];` Will generate satellite map-tiles for LOD 7 starting with row 10|

### `gradMtgStop`
|**Syntax**| |  
|---|---|  
|Description| Stops the current generation of map tiles. (Note: this will not happen immediately, but before generating the next tile)|
|||
|Syntax| **gradMtgStop**
|||
|Return Value| Generation stopped? `<BOOLEAN>`|
|||
|Examples|`gradMtgStop;`|  
  
### `gradMtgCompleteStart`
|**Syntax**| |  
|---|---|  
|Description|Starts generation of map tiles for multiple LODs at once.|
|||
|Syntax| **gradMtgCompleteStart** [minLevelOfDetail, maxLevelOfDetail]
|||
|Parameters|[minLevelOfDetail, maxLevelOfDetail] `<ARRAY>`|
||minLevelOfDetail `<NUMBER>`: Minimum LOD which should be generated|
||maxLevelOfDetail `<NUMBER>`: Maximum LOD which should be generated|
|||
|Return Value| Generation started correctly? `<BOOLEAN>`|
|||
|Examples|`gradMtgCompleteStart [2, 8];`|  
||`gradMtgCompleteStart [1, 7];`|  

### `gradMtgMeta`
|**Syntax**| |  
|---|---|  
|Description| Generate the map's `meta.json`. This file includes various meta data of the map, like map name, map size, gridOffset, etc., which is needed to display the map correctly. 
|||
|Syntax| **gradMtgMeta**
|||
|Return Value| Export successful? `<BOOLEAN>`|
|||
|Examples|`gradMtgMeta;`|  


## Exported Files
All exported files can be found in the `grad_mtg` subdirectory of your Arma 3 installation directory.

## FAQ

### What is a map tile?

According to [ Open Street Map Wiki: Tiles](https://wiki.openstreetmap.org/wiki/Tiles):
> Tiles are rectangular slabs of ceramic affixed in a grid arrangement to your bathroom wall. [...]  
> Here we are much more likely to be talking about map tiles:
> - square bitmap graphics displayed in a grid arrangement to show a map
> - We may also be talking about tiled map data.

### How does grad_mtg work?
In short the map tile generator works like this:
1. The ingame map is opened automatically
2. MTG zooms to the correct part of the map
3. A screenshot of the current extent is taken
4. The screenshot is cropped to only include the correct tile
5. Repeat the process from step 2 until all tiles are exported

### What is a Level of Detail (LOD)?
The LOD specifies how much of the map one tile covers, but why do I need tiles of multiple LODs? Can't I just make 100m x 100m tiles of the map and thats it?  
No, because if you then want to show the whole map (all the way zoomed out) the browser would have to display a lot of tiles. For Stratis (which comparatively is small) that would be 6724 tiles (82x82) and I think we can agree, that that is too much.  
The solution is to show less tiles, which each cover more of the map. For example showing 1000m tiles would mean the browser has to display 81 tiles (instead of the 6724) for the whole of Stratis. 

The LODs this generator exports look like this:  

| Area one tile covers | LOD |
| ---: | :---: |
| 100m x 100m | 8 |
| 200m x 200m | 7 |
| 400m x 400m | 6 |
| 800m x 800m | 5 |
| 1600m x 1600m | 4 |
| 3200m x 3200m | 3 |
| 6400m x 6400m | 2 |
| 12800m x 12800m | 1 |
| 25600m x 25600m | 0 |


### What is a map type? / What is the difference between topographic and satellite?
The best way to explain is probably to just show you:  

| ![](https://i.imgur.com/gvobkES.jpg) | ![](https://i.imgur.com/IWAbGCj.jpg) |
| :---: | :---: |
|Satellite|Topographic|

### Whats the nomenclature of the exported tiles?
Our nomenclature of the tiles matches the one of the [OpenGIS® Web Map Tile Service Implementation Standard](https://www.opengeospatial.org/standards/wmts). 

The tile in the top left corner is 0,0 with the first number specifying the column and the second the row.
![https://i.imgur.com/7Itnufs.png](https://i.imgur.com/7Itnufs.png)

In your Arma installation directory you can find a `grad_mtg` directory, which holds all files exported by grad_mtg. In this directory there is a subdirectory for each map, which holds next to a `meta.json` a `topo` and a `sat` folder.  
From there on the relative path of a tile is `{LOD}/{COL}/{ROW}.png`.  
So for example the topographic tile 0,0 for LOD 8 is `topo/8/0/0.png` and tile 10,0 (tenth tile in first row) would be `topo/8/10/0.png`.

### What's the content of the `meta.json`?
The `meta.json` looks like this:
```jsonc
{
    "displayName": "Stratis",    // Map display name
    "grid": {
        "offsetX": 0,    // Grid origin offset from the left
        "offsetY": 8192  // Grid origin offset from the top
    },
    "layers": [    // Available "Layers" with path and display name
        {
            "name": "Topographic",
            "path": "topo/"
        },
        {
            "name": "Satellite",
            "path": "sat/"
        }
    ],
    "locations": [   // Locations of type nameVillage, nameCity or nameCityCapital
        {
            "name": "Air Station Mike-26",
            "pos": [
                4278.85009765625,
                3855.60009765625,
                -217.83058166503906
            ]
        },
        // [...]
    ],
    "maxLod": 8,                // Maximal available LOD
    "minLod": 2,                // Minimal available LOD
    "worldName": "Stratis",     // Return of worldName script command
    "worldSize": 8192           // Return of worldSize script command
}
```

### How long will an export take?
How long an export takes will vary a lot depending on what exactly you want to do, with the biggest influencing factors being map size and export level of detail.  
But for reference: It took us about 25minutes to export everything for Stratis (topographic + satellite & LODs 2-8)

### But why aren't you just using the EMF export of Arma?
Yes in theory using EMF files would be a lot faster and easier, because you wouldn't have to let Arma run for an hour or so to export a big map. So why not use that?  
We made lots of tests with the [EMF export of arma](https://community.bistudio.com/wiki/ArmA:_Cheats#TOPOGRAPHY), but came to the conclusion that the exported EMF is not of very good quality. Shapes which should be rectangular aren't. Some shapes don't have the correct proportions. Some shapes are completely missing.   
And although you can fix some of those issues after the fact, you can't fix all. Here is an [comparison screenshot](https://i.imgur.com/CSOaWeb.png) which shows the map next to an an already corrected EMF. (The EMF coming directly from Arma is a lot worse.)  
This means we have faulty vector data, which obviously cannot be used to generate correct map tiles. 
