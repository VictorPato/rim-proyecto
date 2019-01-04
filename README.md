# rim-proyecto
Este proyecto busca hacer un detector de fragmentos de comics en los comics completos. Se utilizan descriptores SURF.

## Scrappers
Se utilizan 2 scrappers basados en python para obtener las imagenes de internet de xkcd y C&H. Para ser utilizadas se debe hacer un post-procesamiento para mantener solo imagenes válidas en formatos png, jpg y jpeg.

## Compilación
Utilizar el comando:
```
cmake . && cmake --build .
```
Y luego:
```
make clean && make
```
Esto crea 4 ejecutables. Uno para ver los buenos matches, otro para generar los descriptores a partir de una carpeta, otro para evaluar imagenes una por una y otra para evaluar una carpeta entera de imagenes.
