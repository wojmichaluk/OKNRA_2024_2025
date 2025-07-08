## Zadanie 2
- katalogi _flops_, _times_, _flopso2_ i _timeso2_ zawierają pliki **.csv** z wartościami flopsów (zmierzone przy użyciu `PAPI`) oraz czasów wykonania (w sekundach) dla kolejnych badanych rozmiarów macierzy dla wszystkich wersji programu, przy czym _flops_, _times_ przedstawiają wyniki dla programów bez optymalizacji `-O2`, a _flopso2_, _timeso2_ - z tą optymalizacją
- katalog _graphs_ zawiera wykresy dla powyżej opisanych wartości oraz dwa wykresy porównawcze wszystkich wartości flopsów (**flops_all.png** i **flops_allo2.png**), które są umieszczone w sprawozdaniu
- pliki **.c** zawierają kolejne wersje programu
- plik **ref.txt** to pomocniczy plik referencyjny, służący do sprawdzania poprawności algorytmu
- plik **sprawozdanie.pdf** to sprawozdanie z wykonania zadania
- plik **graphs.py** to pomocniczy plik do generowania wykresów
- plik **zadanie1.pdf** zawiera polecenie zadania

### Uwagi
- przy szacowaniu wydajności jako wartość maksymalną wziąłem wartość dla pojedynczej precyzji, a operowałem na double'ach - stąd tak naprawdę wydajność wynosi dwa razy więcej, około 43.5%
- nie zrobiłem optymalizacji polegającej na _tilingu_, co się negatywnie odbiło zarówno na ocenie, jak i na flopsach (charakterystyczny spadek przy użyciu flagi `-O2` po przekroczeniu cache L3)
- ocena: 43/50 pkt (wszystko poza brakiem tilingu było w porządku) 
