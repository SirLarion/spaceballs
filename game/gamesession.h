#pragma once

#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include <irrKlang.h>

#include "OgreCameraMan.h"
#include "OgreTrays.h"

#include "gui.h"

using namespace Ogre;
using namespace irrklang;

namespace sb
{
    const std::string CAM_0 = "CAM_0";
    const double CAM_MAX_VELO = 20;

    struct PhysicsParams
    {
        float timeStep;         // Yhden fysiikkaloopin iteraation pituus
        int iterations;         // Fysiikkaloopin iteraatioiden m��r� framessa
        float airResistance;    // Palloja hidastava tekij�
        float elasticity;       // Pallojen kimmoisuus
    };

    struct GameParams
    {
        Real mapRadius;         // Pallokent�n s�de
        Real holeRadius;        // Yksitt�isen pussin s�de
        int maxGuides;          // Maksimim��r� ly�nnin avustajan segmenttej�
        bool debugMode;         // Onko debuggaus p��ll�
    };

    // Kameroiden tunnisteet
    enum CamName
    {
        GLOBAL,                 // Menun kamera/lukittu kaukon�kym�
        PLAYER1,                // Vapaasti kontrolloitava kamera
        BALL,                   // Pallosentrinen kamera
        PLAYER2,                // K�ytt�m�tt�m�n�, multiplayerin toisen pelaajan kameraa varten
        Last,                   // Enumeraation l�pi iteroimisen mahdollistava
    };

    // Vuorojen syklin eri vaiheita kuvastava enumeraatio
    enum GameState
    {
        TURN_TRANSITION,        // Pallot pys�htyneet, aloitetaan uusi ly�ntivuoro
        TURN_PLACING_BALL,      // Uusi vuoro, pelaaja asettaa k�sipallon
        TURN_START,             // Uusi vuoro
        TURN_AIM,               // Ly�ntikohta valittu
        BALLS_ACTIVE,           // Palloa ly�ty, palloilla nopeusvektoreita
    };

    class GameSession 
    {
        public:
            GameSession(SceneManager* scn, TrayManager* tray, ISoundEngine* sound, PhysicsParams _pParams, GameParams _gParams, Camera* _cam);
            ~GameSession();

            //Onko peli aloitettu/loppunut
            bool gameStarted;
            bool gameEnded;

            void start();
            void restart();    // Vain Debug modessa
            void createBall(SceneNode* bNode, Entity* ballEnt, int ballNumber);
            bool isBallInsideMap(int b);

            // Fysiikat
            //======================================================//
            void updatePhysics();                                   // Fysiikkaloopi, kutsutaan controllerin renderloopissa
            void hitBall(Vector3 playerPosition);                   // Ly�d��n palloa, ts. annetaan sille alkunopeus
            // Apumetodeita                                         //
            Vector3 bounceFromWall(Vector3 v1, Vector3 v2);         //
            Vector3 projectV1ontoV2(Vector3 v1, Vector3 v2);        //
            //======================================================//


            // Kamerat
            //======================================================//
            std::unique_ptr<OgreBites::CameraMan>                   //
                player;                                             // CameraMan, vastaa suurimmasta osasta kameroiden liikkumisesta
            void switchCamera(int dir);                             // Vaihdetaan syklisesti seuraavaan kameraan kameranimien enumeraatiossa
            void switchCamera(CamName dstCam);                      // Vaihdetaan parametrina annettuun kameraan
            void refreshBallCam();                                  // Siirret��n pallosentrist� kameraa, hy�dyllinen silloin, kun pallo on liikkeess�
            bool isCamInteractive() const;                          // Onko pallokamera/freecam
            const CamName getActiveCam() const;                     //
            Vector3 getCamPosition();                               //
            void updateAimPoint();                                  // P�ivitet��n t�ht�ystilanteessa ly�ntikohta
            bool aimPresses[4] = {false, false, false, false};      // Mitk� nuolin�pp�imist� ovat painettu t�ll� hetkell� j�rj. (UP, DOWN, LEFT, RIGHT)
            //======================================================//


            // Pelilogiikka
            //======================================================//
            const GameState getGameState() const;                   //
            void syncCueballWithPlayer();                           // Siirret��n pelipallo kameran eteen, kun GAME_STATE = TURN_PLACING_BALL
            void increaseHitPower();                                // Nostetaan ly�ntivoimaa, jos powerDelta > 0
            double powerDelta;                                      // Ly�ntivoiman muutos arvojoukossa  {-0.3, 0, 0.3} 
            Player* currentPlayer;                                  // T�ll� hetkell� vuorossa oleva pelaaja
            //======================================================//

        private:

            // ��net
            //======================================================//
            void initSounds(ISoundEngine* sound);                   //
            void makeNoise(Ball* ball, float vol, int num);         // K�sket��n SoundEnginen soittaa palloihin liittyvi� ��ni�
            ISoundEngine* SoundEngine;                              // IrrKlangin rajapinta ��nien soittamiselle
            //======================================================//


            // Pussit
            //======================================================//
            void initHoles(SceneManager* scn);                      //
            Real HOLE_RADIUS;                                       //
            std::vector<SceneNode*> holes;                          // 
            //======================================================//


            // Kamerat
            //======================================================//
            void initCameras(SceneManager* scn);                    //
            bool moveCamera(const Vector3 moveVec);                 // Liikutetaan aktiivista kameraa moveVecin verran
            std::map<CamName, SceneNode*> cameraNodes;              // 
            int numCameras;                                         //
            CamName activeCam;                                      //
            //======================================================//


            // Pallot
            //======================================================//
            void initBalls(SceneManager* scn);                      //
            std::map<int, Ball*> balls;                             // Kaikki pallot
            std::set<int> activeBallIDs;                            // Pallot joita tulee yh� simuloida
            std::set<int> processingBallIDs;                        // Pallot, joiden t�rm�ykset on viel� tarkistamatta
            std::set<int> nonCollidingIDs;                          // Pallot, jotka ei t�rm�� t�m�nhetkisell� fyssa loopin iteraatiolla
            std::vector<std::tuple<int, int, float>>                // Suoritettavat t�rm�ykset j�rjestyksess� 
                collisionQueue;                                     // t�m�nhetkisell� fyssa loopin iteraatiolla
            std::set<int> movingBalls;                              // Pallot jotka ovat t�m�nhetkisell� fyssa loopin iteraatiolla liikkeell�
            std::map<int, Ball*> inActiveBalls;                     // Pallot joita EI en��n simuloida physics loopissa
            int numBalls;                                           //
            //======================================================//


            //Graafinen k�ytt�liittym�
            //======================================================//
            void initGui(OgreBites::TrayManager* trayMgr);          //
            std::unique_ptr<Gui> gui;                               // Graafisen k�ytt�liittym�n hallinnoija
            //======================================================//


            // Pelilogiikka
            //======================================================//
            void ballScored(int b);                                 // Pallo numerolla b (pelipallo = 0) menee pussiin
            bool isHandBall;                                        // Saako seuraava pelaaja k�sipallon (vakiona kyll�, koska helpompi k�sitell� onnistunut ly�nti kuin ep�onnistunut)
            // Pelaajat                                             //
            void initPlayers();                                     // 
            bool playerStaysSame;                                   // Onko t�m�nhetkinen pelaaja my�s seuraavana vuorossa
            void changeTurn();                                      // Vaihdetaan vuoroa (pelaaja voi pysy� samana)
            void changePlayer();                                    // Vaihdetaan pelaajaa ja sitten vuoroa
            void gameWon(Player* winningPlayer);                    // Merkit��n peli p��ttyneeksi
            Player* player1;                                        // P1:ll� vakiopallotyyppin� pisteet
            Player* player2;                                        // P2:lla vakiopallottyypin� raidat
            GameState GAME_STATE;                                   //
            double hitPower;                                        // T�m�nhetkinen ly�ntivoima (1-40)
            //======================================================//


            // Fysiikat
            //======================================================//
            void enqueueCollision(                                  // 
                    int ball, int object, float distance            //
            );                                                      //
            void collideBall(int b1, int object);                   //
            void moveBall(int b);                                   //
            void rotateBall(int b);                                 //
            void decelerateBalls();                                 //
            void checkIfScored(int b);                              //
            Vector3 prevCueballPos;                                 //
            int iterations;                                         //
            float timeStep;                                         //
            float airResistance;                                    //
            float elasticity;                                       //
            bool FIRST_BB_COLLISION = true;                         //
            float VELOCITY_CEILING = 25;                            //
            float VELOCITY_FLOOR = 0.05;                            //
            //======================================================//


            //T�ht�ysviiva
            //======================================================//
            std::vector<std::tuple<Vector3, Vector3, float>>        //
                calculateGuideBeam(                                 //
                        Vector3 startPos, Vector3 dir,              //
                        float startingVelocity                      //
                );                                                  //
            void initAim(SceneManager* scn);                        //
            void updateGuideNodes(                                  // P�ivitet��n t�ht�ysviivan segmentit calculateGuideBeamin palautusarvoilla
                    Vector3 startPos, Vector3 startDir              //
            );                                                      //
            void showGuides();                                      //
            void hideGuides();                                      //
            void showTarget();                                      //
            bool isAiming();                                        // Onko mik��n nuolin�pp�imist� painettuna
            Quaternion getAimRotation();                            // Palauttaa aimBase noden orientaation
            SceneNode* aimBase;                                     // Ly�ntikohdan py�ritt�mist� pelipallon ymp�ri helpottava node (aina pallon keskell�)
            SceneNode* aimPoint;                                    // Ly�ntikohdan node
            std::vector<SceneNode*> guideNodes;                     //
            int MAX_GUIDES;                                         // Maksimim��r� t�ht�ysviivan segmenttej�
            Real GUIDE_LIFETIME = 7;                                // T�ht�ysviivan elinaika eli kuinka pitk�lle lasketaan kimpoiluja. Sama yksikk� kuin timeStep
            Real ACTIVE_GUIDE_COUNT = 2;                            // T�ll� hetkell� laskennan tuloksena saatujen t�ht�ysviivan segmenttejen m��r� 
            //======================================================//


            // Valot
            //======================================================//
            void initLights(SceneManager* scn);                     //
            std::map<std::string, SceneNode*> lightNodes;           //
            //======================================================//


            // Yleisi�
            //======================================================//
            void setupStartPosition();                              //
            Vector3 getLookDir(SceneNode* node);                    // Mihin suuntaan SceneNode osoittaa
            // Pelin vakiot                                         //
            bool IS_DEBUG;                                          // Onko Debug mode p��ll�
            Vector3 ORIGO = Vector3(0, 0, 0);                       //
            Real MAP_RADIUS;                                        // Kent�n s�de
            Real BALL_RADIUS = 1;                                   // Pallon s�de
            //======================================================//
    };
}
