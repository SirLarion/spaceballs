# Spaceballs

Painovoimaton 3D biljardipeli ELEC-A7151 kurssin projektityönä.

## Asennus

Yleisohjeet
https://bkaradzic.github.io/bgfx/build.html

### Linux

1. Lataa Spaceballs ja tarvittavat kirjastot
```
git clone git@version.aalto.fi:tammenm2/spaceballs.git
(tai) git clone https://version.aalto.fi/gitlab/tammenm2/spaceballs.git
cd spaceballs
git submodule init
git submodule update
```

(Aja install.sh tai seuraa ohjeita kohdasta 2. Komennot ovat samat)

2. Asenna BGFX
```
cd submodules/bgfx/
make linux-release64
cd ../..
```
3. Asenna SDL
```
cd submodules/SDL/
mkdir build
./configure --prefix=$(pwd)/build
make
cd ../..
```

4. Luo build kansio ja compilaa projekti
```
mkdir build
cd build
cmake .. && make
```

### Windows

#### SDL Asennus

1. Lataa SDL:n kirjastot https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
2. Tee submodules/SDL/ kansioon uusi kansio build/
3. Siirrä ladatusta zipistä lib/x64/ kansion sisältä tiedostot buildiin
4. Varmista, että spaceballs-projektin ja engine-projektin asetuksissa on RuntimeLibraryssä MT, ei MD tai MDb

#### BGFX Asennus

0. Etsi koneelta Visual Studio Installer

- 0.1 Clickaa 'Modify'

- 0.2 Clickaa 'Individual Components'
    
- 0.3 Hae '141'
    
- 0.4 Valitse kaikki mitä löytyy ja asenna

1. Lataa tarvittavat ohjelmat linkeistä:

-- Make --
- http://gnuwin32.sourceforge.net/downlinks/make.php

-- Muita --
- http://gnuwin32.sourceforge.net/downlinks/coreutils.php
- http://gnuwin32.sourceforge.net/downlinks/libiconv.php
- http://gnuwin32.sourceforge.net/downlinks/libintl.php

-----------------------------------
- 1.2 Kirjoita Windowsin vasemman alakulman hakukenttään "env" (ilman ""-merkkejä). 

- 1.3 Valitse vaihtoehdoista "Edit the system environment variables"

- 1.4 Valitse polku Advanced -> Environment Variables

- 1.5 User variables for "Käyttäjänimi" -kohdasta tuplaklikkaa Variablea nimeltä "Path"

- 1.6 Näytölle avautuu "Edit environment variable"-näkymä. Klikkaa new -> Ja kirjoita avautuneeseen kenttään: "C:\Program Files (x86)\GnuWin32\bin" 
 !!!Tämä toimii jos asensit aikaisemmin mainitut tiedostot installerin määrittämään default lokaatioon!!!

- 1.7 Paina "OK"

-----------------------------------
TÄSTÄ KOHTAA RULLATAAN GIT BASHILLÄ
-----------------------------------

2. Kloonaa Spaceballs haluamaasi kansioon ja siirry siihen:
```
git clone git@version.aalto.fi:tammenm2/spaceballs.git
(tai) git clone https://version.aalto.fi/gitlab/tammenm2/spaceballs.git
cd spaceballs
```


3. Lataa submodulet
```
git submodule init
git submodule update
```

4. Siirry bgfx kansioon ja asenna bgfx
```
cd submodules/bgfx
make
```
Jos tämä tyrii, PATH variablessa on vikaa; ks. 1.2 eteenpäin

----------------------------------------------------------
TÄSTÄ KOHTAA RULLATAAN VISUAL STUDION DEVELOPER CONSOLELLA
----------------------------------------------------------

5. Compilataan bgfx
```
make vs2017-release64
```

6. Siirrytään takaisin root kansioon (spaceballs) ja tehdään build-kansio
```
cd ../..
mkdir build
!!!Visual Studio 2019 tyhjennä toistaiseksi .gitignore-tiedosto, jotta build-kansiota pystytään käsittelemään!!!
```

7. Runataan CMake build-kansiossa
```
cd build
cmake ..
```

8. Avataan Visual Studio
```
start spaceballs.sln
```

9. Konfiguroidaan projekti

- 9.1  Right-clickaa Solution 'spaceballs' oikealla solution explorerissa
    
- 9.2  Valitse 'Properties'
    
- 9.3  Valitse vasemmalta 'Configuration properties'
    
- 9.4  Jos ylhäällä kohdassa 'Platform' lukee 'Win32' clickaa 'Configuration manager' !! JOS EI LUE NIIN SKIPPAA KOHTAAN 9.9 (Visual Studio 2017) / SKIPPAA KOHTAAN 10 (Visual Studio 2019) !!
    
- 9.5  Valitse menusta 'Active solution platform' vaihtoehto '<New...>'
    
- 9.6  Valitse ensimmäiseen kenttään 'x64'
    
- 9.7  Toisessa kentässä kuuluu olla 'Win32' ja checkboxi valittuna
    
- 9.8  Clickaa OK ja sulje ikkunat
    

- 9.9  Right-clickaa spaceballs projektia Solution 'spaceballs': n alla
    
- 9.10 Valitse 'Properties'
    
- 9.11 Avaa vasemmalta 'Linker'
    
- 9.12 Valitse 'All options'
    
- 9.13 Scrollaa listaa ylös, jos 'Additional Options' -kohdassa lukee mitään, poista se
    
    
10. Buildaa projekti painamalla Ctrl+Shift+B, yläpalkista Build -> Build solution tai Right-clickaa ALL_BUILD -> Build

----------------------------------------------------------
OHJELMAN KÄYNNISTÄMINEN
----------------------------------------------------------

11. (Visual Studio 2019) Valmis .exe tiedosto löytyy polusta C:\Users\Aleksi\spaceballs\build\Debug\spaceballs.exe

12. Jos teet muutoksia main.cpp rakenna .exe-tiedosto uudestaan seuraavasti: 
    - 12.1 start spaceballs.sln
    - 12.2 Buildaa projekti painamalla Ctrl+Shift+B, yläpalkista Build -> Build solution tai Right-clickaa ALL_BUILD -> Build
    - 12.3 Käynnistä .exe-tiedosto

13. Siirry Git Bashillä projektin juureen ja tee oma branch
```
git branch "nimi"
git checkout "nimi"
```


