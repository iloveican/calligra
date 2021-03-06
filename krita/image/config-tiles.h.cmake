/* config-tiles.h.  Generated by cmake from config-tiles.h.cmake */

/* Define if you are using the 'old' (Krita 1.5+-style) or 'new' (post-2.O-style) of tile system */
/**
 * 1 - image/tiles
 * 2 - image/tiles_new
 * 3 - image/tiles3
 */
#ifndef CONFIG_TILES_H_
#define CONFIG_TILES_H_


#ifndef USE_TILESYSTEM
#cmakedefine USE_TILESYSTEM @USE_TILESYSTEM@
#endif


#if USE_TILESYSTEM == 1
  #define TILES_DIR(file) <tiles/file>
  #define KIS_TILED_DATA_MANAGER_HEADER TILES_DIR(kis_tileddatamanager.h)

#elif USE_TILESYSTEM == 2
  #define TILES_DIR(file) <tiles_new/file>
  #define KIS_TILED_DATA_MANAGER_HEADER TILES_DIR(kis_tileddatamanager.h)
#elif USE_TILESYSTEM == 3
  #define TILES_DIR(file) <tiles3/file>
  #define KIS_TILED_DATA_MANAGER_HEADER TILES_DIR(kis_tiled_data_manager.h)
#endif

#define KIS_MEMENTO_HEADER TILES_DIR(kis_memento.h)      

#endif /* CONFIG_TILES_H_ */
