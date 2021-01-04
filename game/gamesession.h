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
        int iterations;         // Fysiikkaloopin iteraatioiden määrä framessa
        float airResistance;    // Palloja hidastava tekijä
        float elasticity;       // Pallojen kimmoisuus
    };

    struct GameParams
    {
        Real mapRadius;         // Pallokentän säde
        Real holeRadius;        // Yksittäisen pussin säde
        int maxGuides;          // Maksimimäärä lyönnin avustajan segmenttejä
        bool debugMode;         // Onko debuggaus päällä
    };

    // Kameroiden tunnisteet
    enum CamName
    {
        GLOBAL,                 // Menun kamera/lukittu kaukonäkymä
        PLAYER1,                // Vapaasti kontrolloitava kamera
        BALL,                   // Pallosentrinen kamera
        PLAYER2,                // Käyttämättömänä, multiplayerin toisen pelaajan kameraa varten
        Last,                   // Enumeraation läpi iteroimisen mahdollistava
    };

    // Vuorojen syklin eri vaiheita kuvastava enumeraatio
    enum GameState
    {
        TURN_TRANSITION,        // Pallot pysähtyneet, aloitetaan uusi lyöntivuoro
        TURN_PLACING_BALL,      // Uusi vuoro, pelaaja asettaa käsipallon
        TURN_START,             // Uusi vuoro
        TURN_AIM,               // Lyöntikohta valittu
        BALLS_ACTIVE,           // Palloa lyöty, palloilla nopeusvektoreita
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
            void hitBall(Vector3 playerPosition);                   // Lyödään palloa, ts. annetaan sille alkunopeus
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
            void refreshBallCam();                                  // Siirretään pallosentristä kameraa, hyödyllinen silloin, kun pallo on liikkeessä
            bool isCamInteractive() const;                          // Onko pallokamera/freecam
            const CamName getActiveCam() const;                     //
            Vector3 getCamPosition();                               //
            void updateAimPoint();                                  // Päivitetään tähtäystilanteessa lyöntikohta
            bool aimPresses[4] = {false, false, false, false};      // Mitkä nuolinäppäimistä ovat painettu tällä hetkellä järj. (UP, DOWN, LEFT, RIGHT)
            //======================================================//


            // Pelilogiikka
            //======================================================//
            const GameState getGameState() const;                   //
            void syncCueballWithPlayer();                           // Siirretään pelipallo kameran eteen, kun GAME_STATE = TURN_PLACING_BALL
            void increaseHitPower();                                // Nostetaan lyöntivoimaa, jos powerDelta > 0
            double powerDelta;                                      // Lyöntivoiman muutos arvojoukossa  {-0.3, 0, 0.3} 
            Player* currentPlayer;                                  // Tällä hetkellä vuorossa oleva pelaaja
            //======================================================//

        private:

            // Äänet
            //======================================================//
            void initSounds(ISoundEngine* sound);                   //
            void makeNoise(Ball* ball, float vol, int num);         // Käsketään SoundEnginen soittaa palloihin liittyviä ääniä
            ISoundEngine* SoundEngine;                              // IrrKlangin rajapinta äänien soittamiselle
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
            std::set<int> activeBallIDs;                            // Pallot joita tulee yhä simuloida
            std::set<int> processingBallIDs;                        // Pallot, joiden törmäykset on vielä tarkistamatta
            std::set<int> nonCollidingIDs;                          // Pallot, jotka ei törmää tämänhetkisellä fyssa loopin iteraatiolla
            std::vector<std::tuple<int, int, float>>                // Suoritettavat törmäykset järjestyksessä 
                collisionQueue;                                     // tämänhetkisellä fyssa loopin iteraatiolla
            std::set<int> movingBalls;                              // Pallot jotka ovat tämänhetkisellä fyssa loopin iteraatiolla liikkeellä
            std::map<int, Ball*> inActiveBalls;                     // Pallot joita EI enään simuloida physics loopissa
            int numBalls;                                           //
            //======================================================//


            //Graafinen käyttöliittymä
            //======================================================//
            void initGui(OgreBites::TrayManager* trayMgr);          //
            std::unique_ptr<Gui> gui;                               // Graafisen käyttöliittymän hallinnoija
            //======================================================//


            // Pelilogiikka
            //======================================================//
            void ballScored(int b);                                 // Pallo numerolla b (pelipallo = 0) menee pussiin
            bool isHandBall;                                        // Saako seuraava pelaaja käsipallon (vakiona kyllä, koska helpompi käsitellä onnistunut lyönti kuin epäonnistunut)
            // Pelaajat                                             //
            void initPlayers();                                     // 
            bool playerStaysSame;                                   // Onko tämänhetkinen pelaaja myös seuraavana vuorossa
            void changeTurn();                                      // Vaihdetaan vuoroa (pelaaja voi pysyä samana)
            void changePlayer();                                    // Vaihdetaan pelaajaa ja sitten vuoroa
            void gameWon(Player* winningPlayer);                    // Merkitään peli päättyneeksi
            Player* player1;                                        // P1:llä vakiopallotyyppinä pisteet
            Player* player2;                                        // P2:lla vakiopallottyypinä raidat
            GameState GAME_STATE;                                   //
            double hitPower;                                        // Tämänhetkinen lyöntivoima (1-40)
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


            //Tähtäysviiva
            //======================================================//
            std::vector<std::tuple<Vector3, Vector3, float>>        //
                calculateGuideBeam(                                 //
                        Vector3 startPos, Vector3 dir,              //
                        float startingVelocity                      //
                );                                                  //
            void initAim(SceneManager* scn);                        //
            void updateGuideNodes(                                  // Päivitetään tähtäysviivan segmentit calculateGuideBeamin palautusarvoilla
                    Vector3 startPos, Vector3 startDir              //
            );                                                      //
            void showGuides();                                      //
            void hideGuides();                                      //
            void showTarget();                                      //
            bool isAiming();                                        // Onko mikään nuolinäppäimistä painettuna
            Quaternion getAimRotation();                            // Palauttaa aimBase noden orientaation
            SceneNode* aimBase;                                     // Lyöntikohdan pyörittämistä pelipallon ympäri helpottava node (aina pallon keskellä)
            SceneNode* aimPoint;                                    // Lyöntikohdan node
            std::vector<SceneNode*> guideNodes;                     //
            int MAX_GUIDES;                                         // Maksimimäärä tähtäysviivan segmenttejä
            Real GUIDE_LIFETIME = 7;                                // Tähtäysviivan elinaika eli kuinka pitkälle lasketaan kimpoiluja. Sama yksikkö kuin timeStep
            Real ACTIVE_GUIDE_COUNT = 2;                            // Tällä hetkellä laskennan tuloksena saatujen tähtäysviivan segmenttejen määrä 
            //======================================================//


            // Valot
            //======================================================//
            void initLights(SceneManager* scn);                     //
            std::map<std::string, SceneNode*> lightNodes;           //
            //======================================================//


            // Yleisiä
            //======================================================//
            void setupStartPosition();                              //
            Vector3 getLookDir(SceneNode* node);                    // Mihin suuntaan SceneNode osoittaa
            // Pelin vakiot                                         //
            bool IS_DEBUG;                                          // Onko Debug mode päällä
            Vector3 ORIGO = Vector3(0, 0, 0);                       //
            Real MAP_RADIUS;                                        // Kentän säde
            Real BALL_RADIUS = 1;                                   // Pallon säde
            //======================================================//
    };
}
