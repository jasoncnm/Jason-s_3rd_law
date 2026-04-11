import os
import json

def ReplaceTileSet(mapDir):
    tileMapFiles = os.listdir(mapDir)
    for tileMapFile in tileMapFiles:
        filePath = mapDir + tileMapFile
        with open(filePath, '+r') as file:
            data = json.load(file)
            if 'tilesets' in data:
                for tileset in data['tilesets']:
                    tileset['source'] = '../TileSet/game_tilesets.tsj'
                    print(tileset['source'])
            
            file.seek(0)
            json.dump(data, file, indent=4)
            file.truncate()


print(os.getcwd());

ReplaceTileSet('TileMap/')
ReplaceTileSet('TestLevels/')