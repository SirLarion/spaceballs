#define _USE_MATH_DEFINES

#include "gamesession.h"


using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    // GameSession-olion alustus
    //=======================================================//
    GameSession::GameSession(SceneManager* scn, TrayManager* tray, ISoundEngine* sound, PhysicsParams _pParams, GameParams _gParams, Camera* _cam) :
        numBalls(0), activeCam(GLOBAL), GAME_STATE(TURN_START),
        gameStarted(false), playerStaysSame(true), gameEnded(false), isHandBall(true)
    {
        iterations =    _pParams.iterations;
        timeStep =      _pParams.timeStep;
        airResistance = _pParams.airResistance;
        elasticity =    _pParams.elasticity;

        MAP_RADIUS =    _gParams.mapRadius;
        HOLE_RADIUS =   _gParams.holeRadius;
        MAX_GUIDES =    _gParams.maxGuides;
        IS_DEBUG =      _gParams.debugMode;

        initPlayers();     // Pelaajat ennen palloja, pallojen alustuksessa viittauksia pelaajiin
        initBalls(scn);    // Pallot ennen loppuja, sillä niissä viittauksia palloihin
        initHoles(scn);
        initCameras(scn);
        initLights(scn);
        initSounds(sound);
        initAim(scn);
        initGui(tray);

        cameraNodes[GLOBAL]->attachObject(_cam);

        setupStartPosition();

        std::cout << "GameSession init complete" << std::endl;
    }

    // Vapautetaan GameSessionin "new" komennolla varatut muistit
    GameSession::~GameSession()
    {
        cameraNodes[activeCam]->detachAllObjects();
        for(int i = 0; i < numBalls; i++)
        {
            delete balls[i];
        }
        delete player1;
        delete player2;
    }

    // Pussien alustus
    void GameSession::initHoles(SceneManager* scn)
    {
        Vector3 rot[4];
        for (int i = 0; i < 4; i++)
        {
            // Pallokoordinaatistosta OGRE koordinaatteihin
            rot[i] = Vector3(
                sin(M_PI / 4) * cos(M_PI / 4 + i * (M_PI / 2)),
                sin(M_PI / 4) * sin(M_PI / 4 + i * (M_PI / 2)),
                cos(M_PI / 4)
            );
        }

        Vector3 holeCoords[] =
        {
            MAP_RADIUS*rot[0], MAP_RADIUS*rot[1], MAP_RADIUS*rot[2], MAP_RADIUS*rot[3],
            -MAP_RADIUS*rot[0], -MAP_RADIUS*rot[1], -MAP_RADIUS*rot[2], -MAP_RADIUS*rot[3]
        };

        for(auto h : holeCoords){ 
            SceneNode* hNode = scn->getRootSceneNode()->createChildSceneNode(h);
            hNode->setDirection(-h, Node::TS_PARENT);
            hNode->setScale(Vector3(HOLE_RADIUS, HOLE_RADIUS, HOLE_RADIUS));

            // Debug modessa laitetaan pussien keskipisteisiin meshit, että nähdään, ovatko
            // ne oikeissa paikoissa ja oikean kokoisia
            if(IS_DEBUG)
            {
                Entity* hTarget = scn->createEntity("Target.mesh");
                hNode->attachObject(hTarget);
            }
            holes.push_back(hNode);
        }
    }

    // Pallojen alustus
    void GameSession::initBalls(SceneManager* scn)
    {
        for (int i = 0; i <= 15; i++)
        {
            std::stringstream fnamestream;
            fnamestream << "NUM" << i << ".mesh"; 
            std::string meshFile = fnamestream.str();
            SceneNode* bNode = scn->getRootSceneNode()->createChildSceneNode(Vector3(-20 + 2 * i, 0, 0));
            Entity* ballEnt = scn->createEntity(meshFile);
            createBall(bNode, ballEnt, i);
        }
        prevCueballPos = balls[0]->getPos();
    }

    // Kameroiden alustus
    void GameSession::initCameras(SceneManager* scn)
    {
        for (int i = GLOBAL; i != Last; i++)
        {
            CamName name = static_cast<CamName>(i);
            SceneNode* _camNode = scn->getRootSceneNode()->createChildSceneNode();
            _camNode->setPosition(0, 35, 120);
            _camNode->lookAt(Vector3::ZERO, Node::TS_PARENT);

            numCameras++;
            cameraNodes[name] = _camNode;
        }
        cameraNodes[BALL]->setOrientation(Quaternion(1, 0, -1, 0));
        player = std::make_unique<CameraMan>(cameraNodes[PLAYER1]);
        player->setTarget(balls[0]->getNode());
        player->setTopSpeed(CAM_MAX_VELO);
        player->setStyle(CS_FREELOOK);
    }

    // Valojen alustus
    void GameSession::initLights(SceneManager* scn)
    {
        Light* directional = scn->createLight();
        directional->setType(Light::LT_DIRECTIONAL);
        SceneNode* _directionalNode = scn->getRootSceneNode()->createChildSceneNode();
        _directionalNode->pitch(Degree(-20));
        _directionalNode->yaw(Degree(-20));
        _directionalNode->attachObject(directional);
    }

    // Äänien alustus
    void GameSession::initSounds(ISoundEngine* sound)
    {
        SoundEngine = sound;
    }

    // Tähtäyskokonaisuuden alustus
    void GameSession::initAim(SceneManager* scn)
    {
        // Määritellään lyöntikohdan piste
        aimBase = scn->getRootSceneNode()->createChildSceneNode();
        aimPoint = aimBase->createChildSceneNode();
        aimPoint->translate(Vector3(BALL_RADIUS, 0, 0), Node::TS_LOCAL);
        Entity* targetEnt = scn->createEntity("Target.mesh");

        aimPoint->attachObject(targetEnt);
        aimPoint->setScale(Vector3(0.2));
        aimPoint->setVisible(false);

        //Määritellään tähtäysviiva, enimmillään MAX_GUIDES segmenttiä
        for (int i = 0; i < MAX_GUIDES-1; i++)
        {
            Entity* guide = scn->createEntity("Guide.mesh");
            SceneNode* _guideNode = scn->getRootSceneNode()->createChildSceneNode();
            _guideNode->setVisible(false);
            _guideNode->attachObject(guide);
            guideNodes.push_back(_guideNode);
        }
        Entity* guideLast = scn->createEntity("TaperedGuide.mesh");
        SceneNode* _lastNode = scn->getRootSceneNode()->createChildSceneNode();
        _lastNode->setVisible(false);
        _lastNode->attachObject(guideLast);
        guideNodes.push_back(_lastNode);

        // Viiva aluksi piiloon
        hideGuides();
    }

    // GUIn alustus
    void GameSession::initGui(TrayManager* trayMgr)
    {
        gui = std::make_unique<Gui>(currentPlayer);
        hitPower = 20;
        gui->initGui(trayMgr, hitPower);
    }

    // Pelaajien alustus
    void GameSession::initPlayers()
    {
        player1 = new Player("Player1", "Solids", 1);
        player2 = new Player("Player2", "Stripes", 2);

        currentPlayer = player1;
    }

    // Pallojen asettaminen alkuasetelmaan
    void GameSession::setupStartPosition()
    {
        Real xDist = 22;
        Real offset = 0.1;

        Real r = BALL_RADIUS;
        Real bD = 2 * r + offset; // ball diameter
        float lH = sqrt(pow(bD, 2) - pow(r, 2)) + offset; // layer height

        // Alkuasetelma pyramidin muotoinen. Symmetria aiheuttaa tilanteita, joissa pallo
        // osuu kahteen muuhun palloon samalla fysiikkaloopin iteraatiolla ja pallot jäävät
        // toisiinsa kiinni sisäkkäin. Symmetria rikotaan pallojen sijaintien offseteillä
        Vector3 startPositions[]
        {
            Vector3(xDist - 2 * lH - bD, 0, 0),

            Vector3(xDist - 2 * lH, 0, 0),

            Vector3(xDist-lH, bD/2, -bD/2), Vector3(xDist-lH-offset, bD/2, bD/2),
            Vector3(xDist-lH-2*offset, -bD/2, -bD/2), Vector3(xDist-lH-3*offset, -bD/2, bD/2),

            Vector3(xDist-4*offset,  bD,  -bD), Vector3(xDist-3*offset,  bD, 0), Vector3(xDist-2*offset,  bD, bD),
            Vector3(xDist - offset,   0,  -bD), Vector3(xDist,            0, 0), Vector3(xDist + offset,   0, bD),
            Vector3(xDist+2*offset, -bD,  -bD), Vector3(xDist+3*offset, -bD, 0), Vector3(xDist+4*offset, -bD, bD)
        };

        balls[0]->getNode()->setPosition(-xDist, 0, 0);
        for (int b = 1; b < numBalls; b++)
        {
            Vector3 pos = startPositions[b - 1];
            balls[b]->getNode()->setPosition(pos);
        }
    }

    // Apumetodi yksittäisen pallon luomiseen. Metodi on julkinen, jotta sitä voidaan kutsua
    // debug modessa suoraan kontrollerista
    void GameSession::createBall(SceneNode* bNode, Entity* ballEnt, int ballNumber)
    {
        activeBallIDs.insert(ballNumber);
        Ball* b = new Ball(bNode, ballNumber);
        b->setVelocity(Vector3::ZERO);
        b->attach(ballEnt);
        balls[ballNumber] = b;
        numBalls++;

        //Asetetaan pallo sille kuuluvan pelaajan listaan
        //8. pallo asetetaan molemmille pelaajille
        if (ballNumber > 0)
        {
            // Kaikki pistepallot < 9, Raitapallot >= 9
            if (ballNumber < 9)
            {
                player1->addBall(balls[ballNumber]);
            }
            else
            {
                player2->addBall(balls[ballNumber]);
            }
        }
    }

    // Onko pallo numerolla b pelikentän sisällä? Toistaiseksi käytetään vain käsipallon asettamisessa
    bool GameSession::isBallInsideMap(int b)
    {
        Vector3 pos = balls[b]->getPos();
        return pos.distance(Vector3::ZERO) < MAP_RADIUS-BALL_RADIUS;
    }

    void GameSession::start()
    {
        switchCamera(BALL);
        GAME_STATE = TURN_START;
        gameStarted = true;
    }

    // Debug moden metodi, laitetaan pallot (jotakuinkin) alkuasemiin
    void GameSession::restart()
    {
        setupStartPosition();
        for (int i = 0; i < numBalls; i++) { balls[i]->setVelocity(Vector3::ZERO); }
        activeBallIDs.insert(0);
    }
    //=======================================================//

    // Äänien käsittely
    //=======================================================//

    // Käsketään SoundEnginen soittaa ääntä pallon ball sijainnissa voimakkuudella vol.
    // num määrittää, mikä äänitiedosto soitetaan
    void GameSession::makeNoise(Ball* ball, float vol, int num)
    {
        std::string root_path = "";
        root_path.append(PROJECT_PATH);
        std::string musicfile_hit = root_path + "/sounds/pool_os2.wav";
        std::string musicfile_hit2 = root_path + "/sounds/os3.wav";
        const char* cstr1 = musicfile_hit.c_str();
        const char* cstr2 = musicfile_hit2.c_str();
        auto b = player->getCamera()->getPosition();

        ISound* eff = NULL;

        switch (num)
        {
            case 1:
                eff = SoundEngine->play3D(cstr1, vec3df(ball->getPos().x - b.x, ball->getPos().y - b.y, ball->getPos().z - b.z), false, false, true);
                break;

            case 2:
                eff = SoundEngine->play3D(cstr2, vec3df(ball->getPos().x - b.x, ball->getPos().y - b.y, ball->getPos().z - b.z), false, false, true);
                break;
        }
        if (eff) {
            eff->setMinDistance(5.0f);
            eff->setVolume(vol);
        }
    }
    //=======================================================//

    // Kameroiden käsittely
    //=======================================================//

    // Vaihdetaan kameraa syklisesti suuntaan dir, 1: eteenpäin, -1: taaksepäin
    // 0: vaihdetaan pallosentrisen ja freecamin välillä
    void GameSession::switchCamera(int dir)
    {
        CamName dstCam = GLOBAL;
        if (dir == 0)
            dstCam = activeCam == PLAYER1 ? BALL : PLAYER1;
        else
        {
            dstCam = static_cast<CamName>(static_cast<int>(activeCam + dir) % Last);
        }
        switchCamera(dstCam);
    }

    // Konkreettisen kameran vaihdon tekevä overloadattu metodi
    void GameSession::switchCamera(CamName dstCam)
    {
        // Estetään kameran vaihtaminen käsipallon asettamisen aikana
        if(GAME_STATE != TURN_PLACING_BALL)
        {
            auto cam0 = cameraNodes[activeCam]->detachObject(CAM_0);
            cameraNodes[dstCam]->attachObject(cam0);
            if(dstCam == BALL)
            {
                refreshBallCam();
                SceneNode* cam = cameraNodes[BALL]; 
                Vector3 ballPos = balls[0]->getPos();
                // Pallosentriseen kameraan vaihtaessa asetetaan kamera tietylle etäisyydelle
                // pallosta (10 yksikköä)
                cam->setPosition(ballPos);
                cam->translate(-10*getLookDir(cam), Node::TS_PARENT);

                // Vaihdetaan CameraMan pyörimään kohteensa ympärillä
                player->setStyle(CS_ORBIT);
                player->setCamera(cam);

                // Hack, normaalisti OGREn CameraMan liikuttaa orbit kameraa vain, kun
                // hiiren vasen nappi on pohjassa. Annetaan CameraManille dummy-hiirenklikkaustapahtuma,
                // että se luulee napin olevan painettu
                MouseButtonEvent evt{0, 0, MOUSEBUTTONDOWN, BUTTON_LEFT, 1};
                player->mousePressed(evt);

            }
            else if(dstCam == PLAYER1)
            {
                //Otetaan edellisen kameran orientaatio, että kameran vaihto on sulava
                cameraNodes[PLAYER1]->setOrientation(cameraNodes[activeCam]->getOrientation());
                cameraNodes[PLAYER1]->setPosition(getCamPosition());
                if(activeCam == BALL)
                {
                    // "Päästetään irti" pallosentriseen vaihtaessa luotu napinpainallus
                    MouseButtonEvent evt{0, 0, MOUSEBUTTONUP, BUTTON_LEFT, 1};
                    player->mouseReleased(evt);
                }

                // Vaihdetaan CameraMan liikkumaan vapaasti
                player->setStyle(CS_FREELOOK);
                player->setCamera(cameraNodes[PLAYER1]);

                // Jos vaihto tapahtui vuoron alussa, mahdollistetaan pallon lyönti vaihtamalla
                // TURN_AIM tilaan ja näytetään tähtäysviiva
                if(GAME_STATE == TURN_START)
                {
                    GAME_STATE = TURN_AIM;
                    showGuides();
                }
            }
            activeCam = dstCam;
        }
    }

    // Pallosentrisen kameran päivitys, käytetään pelipallon liikkuessa, että kamera
    // pysyy perässä
    void GameSession::refreshBallCam()
    {
        Vector3 cueballPos = balls[0]->getPos();
        Vector3 delta = cueballPos - prevCueballPos;
        prevCueballPos = cueballPos;
        moveCamera(delta);
    }

    // Liikutetaan aktiivista kameraa (paitsi, jos kamera on GLOBAL)
    bool GameSession::moveCamera(const Vector3 moveVec)
    {
        if (moveVec != Vector3::ZERO)
        {
            if (activeCam == GLOBAL)
            {
                return false;
            }
            cameraNodes[activeCam]->translate(moveVec, Node::TS_PARENT);
            return true;
        }
        return false;
    }

    Vector3 GameSession::getCamPosition() { return cameraNodes[activeCam]->getPosition(); }

    // Voiko pelaaja vuorovaikuttaa kameran kanssa? (ts. vain freecam ja pallosentrinen kamera)
    bool GameSession::isCamInteractive() const { return activeCam == PLAYER1 || activeCam == BALL; }

    const CamName GameSession::getActiveCam() const { return activeCam; }

    // Palautetaan parametrina annetun noden globaali katsomissuunta
    Vector3 GameSession::getLookDir(SceneNode* node)
    {
        Matrix3 M = node->getLocalAxes();
        return M * Vector3(0, 0, -1);
    }

    // Onko, jokin nuolinäppäimistä painettuna (eli muuttaako pelaaja pallon tähtäystä)
    bool GameSession::isAiming() { return aimPresses[0]||aimPresses[1]||aimPresses[2]||aimPresses[3]; }

    // Palautetaan tähtäyksestä tällä hetkellä aiheutuva rotaatio quaternionina
    Quaternion GameSession::getAimRotation()
    {
        float aimRotSpeed = 0.001;
        Radian rot;
        Vector3 rotAxis;

        for(int i = 0; i < 4; i++)
        {
            // jokaista painettua nuolinäppäintä kohti päivitetään kokonaisrotaatio
            // nuolinäppäimen aiheuttamalla muutoksella
            if(aimPresses[i])
            {
                switch(i)
                {
                    case 0:
                        rot = Radian(-aimRotSpeed);
                        rotAxis = Vector3::UNIT_X;
                        break;

                    case 1:
                        rot = Radian(aimRotSpeed);
                        rotAxis = Vector3::UNIT_X;
                        break;

                    case 2:
                        rot = Radian(-aimRotSpeed);
                        rotAxis = Vector3::UNIT_Y;
                        break;

                    case 3:
                        rot = Radian(aimRotSpeed);
                        rotAxis = Vector3::UNIT_Y;
                        break;
                }
            }
        }
        return Quaternion(rot, rotAxis);
    }

    // Päivitetään lyöntikohta ja vastaavasti tähtäysviivan segmenttien suunnat ja paikat
    void GameSession::updateAimPoint()
    {
        if(GAME_STATE == TURN_START) showTarget();
        Vector3 cueballPos = balls[0]->getPos();

        aimBase->setPosition(cueballPos);
        aimPoint->setPosition(Vector3::ZERO);

        // Jos pelaaja ei tällä hetkellä muuta tähtäystä, lasketaan lyöntikohta
        // pallosentrisen kameran osoitussuunnasta, muulloin selvitetään, kuinka paljon
        // lyöntikohtaa pitäisi siirtää
        if(!isAiming() || activeCam == BALL)
            aimBase->setOrientation(cameraNodes[BALL]->getOrientation());
        else
        {
            Quaternion aimRot = getAimRotation();
            aimBase->rotate(aimRot);
            cameraNodes[BALL]->setOrientation(aimBase->getOrientation());
        }

        Vector3 aimDirection = getLookDir(aimBase).normalisedCopy();
        aimPoint->translate(
                -BALL_RADIUS * aimPoint->convertWorldToLocalDirection(aimDirection, false), 
                Node::TS_PARENT
        );

        // Päivitetään loppuun tähtäysviiva alkaen pelipallon sijainnista aimBase noden suuntaan
        updateGuideNodes(cueballPos, aimDirection);
    }

    // Tähtäysviivan päivittäminen. Segmenttien uudet sijainnit, suunnat ja pituudet lasketaan
    // calculateGuideBeam metodilla
    void GameSession::updateGuideNodes(Vector3 startPos, Vector3 startDir)
    {
        float hitVelocity = ((hitPower/2)*startDir).length();

        auto guidesCalculated = calculateGuideBeam(startPos, startDir, hitVelocity);
        int i = 0;

        // Käydään läpi guideNodeista yksi vähemmän kuin aktiivisten segmenttien määrä ja
        // päivitetään niiden attribuutit calculateGuideBeamin paluuarvoilla
        while (i < ACTIVE_GUIDE_COUNT -1)
        {
            guideNodes[i]->setPosition(std::get<0>(guidesCalculated[i]));
            guideNodes[i]->setDirection(std::get<1>(guidesCalculated[i]), Node::TS_PARENT);
            guideNodes[i]->setScale(Vector3(0.05, 0.05, std::get<2>(guidesCalculated[i])));
            i++;
        }

        // Viimeinen aktiivisista segmenteistä on aina guideNodes listan viimeinen, sillä sen
        // mesh on kaventuva ja luo tähtäysviivan "kärjen"
        guideNodes[MAX_GUIDES-1]->setPosition(std::get<0>(guidesCalculated[i]));
        guideNodes[MAX_GUIDES-1]->setDirection(std::get<1>(guidesCalculated[i]), Node::TS_PARENT);
        guideNodes[MAX_GUIDES-1]->setScale(Vector3(0.05, 0.05, std::get<2>(guidesCalculated[i])));

        while (i < MAX_GUIDES)
        {
            guideNodes[i]->setVisible(false);
            i++;
        }
        showGuides();
    }

    void GameSession::showTarget() { aimPoint->setVisible(true); }

    // Näytetään guiden aktiiviset segmentit 
    void GameSession::showGuides()
    {
        for (unsigned int i = 0; i < ACTIVE_GUIDE_COUNT; i++) 
        {
            if(i == ACTIVE_GUIDE_COUNT-1)
                guideNodes[MAX_GUIDES-1]->setVisible(true);
            else
                guideNodes[i]->setVisible(true); 
        }
    }

    void GameSession::hideGuides()
    {
        aimPoint->setVisible(false);
        for(auto g : guideNodes){ g->setVisible(false); }
    }

    // Palauttaa vectorin, joka sisältää ohjeet asianmukaisten tähtäysviivan piirtämiseen
    // formaatissa (startPos, dir, length)
    std::vector<std::tuple<Vector3, Vector3, float>>
        GameSession::calculateGuideBeam(Vector3 startPos, Vector3 dir, float startingVelocity)
    {
        std::vector<std::tuple<Vector3, Vector3, float>> guideBeams;
        float maxLength = startingVelocity * GUIDE_LIFETIME;
        std::set<int> ballIDs = activeBallIDs;
        ballIDs.erase(0);

        float length = 0;
        Vector3 currentPos = startPos;
        Vector3 currentDir = dir / dir.length(); //pakotetaan dir yksikkövektoriksi laskuja varten

        Vector3 currentPosToBall;
        Vector3 alongDir;
        Vector3 alongDirToBall;
        Vector3 nextPos;
        Vector3 posToNextPos;
        Vector3 nextPosToBall;

        float tempLength = MAP_RADIUS * 2;
        Radian tempAngle;
        bool ballBounce = false;
        int bounceBall = -1;
        bool done = false;
        int i = 0;
        while (!done)
        {
            for (auto it : ballIDs)
            {
                currentPosToBall = balls[it]->getPos() - currentPos;
                if (currentDir.dotProduct(currentPosToBall) > 0) {
                    alongDir = projectV1ontoV2(currentPosToBall, currentDir);
                    alongDirToBall = currentPosToBall - alongDir;
                    if (alongDirToBall.length() < BALL_RADIUS * 2) {
                        // pythagooralla varsinainen sijainti jossa osutaan toiseen palloon
                        alongDir = alongDir * (1 - std::sqrt(std::pow(BALL_RADIUS * 2, 2) - std::pow(alongDirToBall.length(), 2)) / alongDir.length());
                        //onko pallo säteen reitillä? onko ensimmäinen johon törmättäisiin?
                        if (alongDir.length() < tempLength)
                        {
                            ballBounce = true;
                            nextPos = currentPos + alongDir;
                            nextPosToBall = currentPosToBall - alongDir;
                            tempLength = alongDir.length();
                            bounceBall = it;
                        }
                    }
                }
            }
            //päivitetään currentPos ja currentDir seuraavan pätkän laskemista varten

            if (ballBounce) //osutaanko palloon?
            {
                //poistetaan pallo ohjurille näkyvistä, sillä siihen on jo törmätty,
                //jolloin uuden törmäyksen määrittäminen tästä pallosta on, joka tapauksessa epäluotettavaa
                //etukäteen analyyttisesti
                ballIDs.erase(bounceBall);
                if (length + tempLength > maxLength) { tempLength = maxLength - length; done = true; }
                length = length + tempLength;
                guideBeams.push_back(std::make_tuple(currentPos, currentDir, tempLength * 0.5));

                currentPos = nextPos;
                currentDir = currentDir - projectV1ontoV2(currentDir, nextPosToBall);
                currentDir = currentDir / currentDir.length();
            }
            //jos ei, niin seuraavan säteen suunta määrittyy kimpoomisella kentän seinästä
            else
            {
                //lasketaan kohta, jossa currentPos + currentDir * (vakio) osuu kentän seinään ratkasemalla (vakio)
                tempLength = currentPos.length();
                tempAngle = currentPos.angleBetween(currentDir);
                posToNextPos = currentDir * (-tempLength * Ogre::Math::Cos(tempAngle) +
                    std::sqrt((MAP_RADIUS - tempLength * Ogre::Math::Sin(tempAngle)) * (MAP_RADIUS + tempLength * Ogre::Math::Sin(tempAngle))));
                //checkataan guide pätkien yhteenlaskettu pituus
                tempLength = posToNextPos.length();
                if (length + tempLength > maxLength) { tempLength = maxLength - length; done = true; }
                length = length + tempLength;
                guideBeams.push_back(std::make_tuple(currentPos, currentDir, tempLength * 0.5));

                currentPos = currentPos + posToNextPos;
                currentDir = bounceFromWall(currentDir, currentPos);
                currentDir = currentDir / currentDir.length();
            }
            tempLength = MAP_RADIUS * 2;
            ballBounce = false;
            i++;
            //rajataan laskettavien guidejen määrä, jottei graffa puolella indeksöidä yli 
            //vertaamalla liian isoon ACTIVE_GUIDE_COUNT
            if (i >= MAX_GUIDES) done = true;
        }
        ACTIVE_GUIDE_COUNT = i;
        return guideBeams;
    }
    //=======================================================//

    // Pelilogiikan käsittely
    //=======================================================//
    void GameSession::ballScored(int b)
    {

        if (b == 0)
            isHandBall = true;
        else if (b == 8)
        {
            //Tarkistetaan onko nykyisen pelaajan pallopussissa jäljellä vain 8. pallo
            if (currentPlayer->getBallsLeft() > 1)
            {
                //Nykyinen pelaaja on hävinnyt pelin
                if (currentPlayer == player1)
                    gameWon(player2);
                else
                    gameWon(player1);
            }
            else
            {
                //Nykyinen pelaaja on voittanut pelin
                gameWon(currentPlayer);
            }
        }
        else if (b < 9)
        {
            player1->ballPotted(b);
            //Jos kyseessä oli player1 vuoro, jatketaan vuoroa ilman että vaihdetaan pelaajaa
            if (currentPlayer == player1)
                playerStaysSame = true;
        }
        else
        {
            player2->ballPotted(b);
            //Jos kyseessä oli player2 vuoro, jatketaan vuoroa ilman että vaihdetaan pelaajaa
            if (currentPlayer == player2)
                playerStaysSame = true;
        }

    }

    // Jos tämänhetkisellä pelaajalla on käsipallo, päivitetään jatkuvasti pelipallon
    // sijainti pelaajan kameran eteen
    void GameSession::syncCueballWithPlayer()
    {
        Vector3 camDirection = getLookDir(cameraNodes[PLAYER1]);
        balls[0]->getNode()->setPosition(cameraNodes[PLAYER1]->getPosition());
        balls[0]->move(10*camDirection);
    }

    // Vaihdetaan vuoroa
    void GameSession::changeTurn()
    {
        if(isHandBall)
        {
            balls[0]->getNode()->setVisible(true);
            activeBallIDs.insert(0);
            GAME_STATE = TURN_PLACING_BALL;
            switchCamera(PLAYER1);
            syncCueballWithPlayer();
        }
        else
        {
            GAME_STATE = TURN_START;
            switchCamera(BALL);
            isHandBall = true;
        }

    }

    // Vaihdetaan pelaajaa ja lopuksi vuoroa
    void GameSession::changePlayer()
    {
        if (currentPlayer == player1) {
            currentPlayer = player2;
            gui->changePlayer(currentPlayer);
        }
        else
        {
            currentPlayer = player1;
            gui->changePlayer(currentPlayer);
        }
        changeTurn();
    }

    // Merkataan peli päättyneeksi
    void GameSession::gameWon(Player* winningPlayer)
    {
        currentPlayer = winningPlayer;
        gameEnded = true;
    }

    const GameState GameSession::getGameState() const { return GAME_STATE; }
    //=======================================================//

    // Fysiikoiden käsittely
    //=======================================================//
    void GameSession::updatePhysics()
    {
        for (int i = 0; i < iterations; i++) {
            nonCollidingIDs = activeBallIDs;
            processingBallIDs = activeBallIDs;
            float distTemp;
            for (auto it1 : activeBallIDs) {
                Ball* b1 = balls[it1];
                Vector3 b1pos = b1->getPos();
                distTemp = b1pos.distance(ORIGO);
                //tarkistetaan törmääkö b1 kentän seinään
                if (distTemp > MAP_RADIUS) {
                    enqueueCollision(it1, -1, distTemp - MAP_RADIUS);
                    nonCollidingIDs.erase(it1);
                }
                processingBallIDs.erase(it1);
                //tarkistetaan törmääkö b1 muihin palloihin
                for (auto it2 : processingBallIDs) {
                    distTemp = balls[it2]->getPos().distance(b1pos);
                    if (distTemp < BALL_RADIUS * 2) {
                        enqueueCollision(it1, it2, distTemp);
                        nonCollidingIDs.erase(it1); nonCollidingIDs.erase(it2);
                    }
                }
            }
            for (auto it : nonCollidingIDs) {
                moveBall(it);
            }
            for (auto it : collisionQueue) {
                collideBall(std::get<0>(it), std::get<1>(it));
            }
            collisionQueue.clear();
        }
        decelerateBalls();
        //tämä if haara suoritetaan, kun pelin fysikaalinen tila on seisahtanut
        if (movingBalls.size() == 0 && GAME_STATE == BALLS_ACTIVE) {
            GAME_STATE = TURN_TRANSITION;
            FIRST_BB_COLLISION = true;
            // Jos pelaaja ei ole saanut pussitettua oikeaa palloa, tai lyönti on virheellinen,
            // vaihdetaan pelaajaa
            if (gameStarted && (!playerStaysSame || isHandBall))
            {
                //Vaihdetaan pelaajaa
                changePlayer();
                playerStaysSame = true;
            }
            else
            {
                changeTurn();
            }
        }
    }
    // Asettaa havaitun törmäyksen jonoon. Jonon järjestys määrittyy, sen mukaan kuinka paljon paljon törmäyksen objektit
    // ovat sisäkkäin. Tämä toimii pienellä timeStepillä varsin hyvin lineaarisen interpolaation siasta.
    void GameSession::enqueueCollision(int ball, int object, float distance)
    {
        std::tuple<int, int, float> entry = std::make_tuple(ball, object, distance);
        bool done = false;
        size_t QLen = collisionQueue.size();
        auto it = collisionQueue.begin();
        int i = 0;
        std::tuple<int, int, float> temp;

        while (i < QLen && !done) {
            temp = collisionQueue[i];
            if (distance > std::get<2>(collisionQueue[i])) {
                done = true;
            }
            it++;
            i++;
        }
        collisionQueue.insert(it, entry);
    }
    // Käsitellään yksittäinen törmäys. Klassinen täysin elastinen törmäys. Pallojen massat ovat kaikki samat,
    // ja kentän massa ääretön, joten uudet nopeudet saadaan laskettua yksinkertaisella vektori matematiikalla.
    void GameSession::collideBall(int b1, int object)
    {
        Ball* ball = balls[b1];

        makeNoise(ball, 0.1, 1);
        //törmäys pallon b1 ja toisen pallon välillä
        if (object >= 0) {
            if (FIRST_BB_COLLISION && 
                ( (object < 8 && currentPlayer->getID() == 1) || 
                  (object > 8 && currentPlayer->getID() == 2) ||
                  (object == 8 && currentPlayer->getBallsLeft() == 1) )
                ) {
                isHandBall = false;
                FIRST_BB_COLLISION = false;
            }
            Ball* ball2 = balls[object];
            Vector3 B1toB2 = ball->getPos() - ball2->getPos();
            Vector3 velB1 = ball->getVelocity();
            Vector3 velB2 = ball2->getVelocity();
            Vector3 velB1proj = projectV1ontoV2(velB1, B1toB2);
            Vector3 velB2proj = projectV1ontoV2(velB2, B1toB2);

            ball->setVelocity((velB1 - velB1proj + velB2proj) * elasticity);
            ball2->setVelocity((velB2 - velB2proj + velB1proj) * elasticity);

            moveBall(b1); moveBall(object);
        }
        //törmäys pallon b1 ja objektin -1, eli seinän välillä
        else {
            checkIfScored(b1);
            ball->setVelocity(bounceFromWall(ball->getVelocity(), ball->getPos()) * elasticity);
            moveBall(b1);
        }
    }
    //liikuttaa pallon Ogre::node objektia pallon nopeuden mukaisesti
    void GameSession::moveBall(int b)
    {
        Ball* ball = balls[b];
        ball->move((ball->getVelocity() * timeStep));
    }

    // Käy läpi kaikki pallot ja hidastaa niitä naivin vakio-ilmanvastuksen mukaisesti, mikäli pallo ei ole törmäämässä.
    // Törmääviä palloja ei hidasteta, sillä se parantaa törmäysten luotettavuutta, eikä eroa huomaa pelatessa.
    void GameSession::decelerateBalls()
    {
        Ball* ball;
        Vector3 velocity;
        Real magnitude;
        for (auto it : activeBallIDs) {
            ball = balls[it];
            velocity = ball->getVelocity();
            magnitude = velocity.length();
            if (magnitude < VELOCITY_FLOOR) {//pallon nopeus on mitätön, joten se asetetaan nollaan
                ball->setVelocity(ORIGO);
                movingBalls.erase(it);
            }
            else {
                if (magnitude > VELOCITY_CEILING) {//jos pallon nopeus on liian suuri, se asetetaan hieman alle maksimin
                    ball->setVelocity((velocity / magnitude) * (VELOCITY_CEILING - 0.1));
                }
                movingBalls.insert(it);
            }
        }
        for (auto it : nonCollidingIDs) {
            ball = balls[it];
            ball->setVelocity(ball->getVelocity() * airResistance);
        }
    }
    //kutsutaan palloille, jotka ovat törmäämässä kentän seinään. Jos pallo on tarpeeksi lähellä jotain reikää,
    //se ei kimpoa seinästä vaan menee pussiin, jolloin kutsutaan ballScoredia ja siistitään pallo pois fysiikoista
    void GameSession::checkIfScored(int b)
    {
        Ball* ball = balls[b];
        for (auto it : holes) {
            Vector3 holePos = it->getPosition();
            if (ball->getPos().distance(holePos) < HOLE_RADIUS) {
                ball->setVelocity(ORIGO);
                movingBalls.erase(b);
                activeBallIDs.erase(b);
                ballScored(b);
                ball->getNode()->setVisible(false);
                return;
            }
        }
    }

    // Muutetaan lyöntivoimaa
    void GameSession::increaseHitPower()
    {
        // Jos lyöntivoima saavuttaa maksimiarvonsa palautetaan lyöntivoimakkuus yhteen
        double newPower = hitPower + powerDelta;
        if (newPower > 40) {
            hitPower = 1;
        }
        // Jos taas voima menee alle yhden laitetaan se maksimiksi
        else if(newPower < 1) {
            hitPower = 40;
        }
        else {
            hitPower = newPower;
        }
        //Päivitetään graafista käyttöliittymää
        gui->updateHitPower(hitPower);
    }

    // Vastaanotetaan pallon lyöntikäsky ja riippuen pelitilanteesta reagoidaan siihen
    void GameSession::hitBall(Vector3 playerPosition)
    {
        // Käsipalloa asettaessa laitetaan pallo paikalleen, jos se on kentän sisällä
        if(GAME_STATE == TURN_PLACING_BALL)
        {
            if(isBallInsideMap(0))
            {
                GAME_STATE = TURN_START;
                switchCamera(BALL);
            }
        }
        else
        {
            // Muulloin, jos pelaaja on tähtäysvaiheessa (tähtäysviiva näkyy)
            // asetetaan pallolle nopeusvektori lyöntivoimakkuuden perusteella
            if(GAME_STATE == TURN_AIM)
            {
                makeNoise(balls[0], 1, 2);
                GAME_STATE = BALLS_ACTIVE;
                balls[0]->setVelocity(hitPower * getLookDir(aimBase));
                hideGuides();
                playerStaysSame = false;
            }   
        }
    }
    //laskee vektorin v1 "heijastuksen" kentän seinästä, kun sen lähtöpiste on v2
    Vector3 GameSession::bounceFromWall(Vector3 v1, Vector3 v2)
    {
        return v1 - (v2 * v1.dotProduct(v2) / (MAP_RADIUS * MAP_RADIUS)) * 2;
    }
    // laskee vektorin v1 projektion vektorille v2
    Vector3 GameSession::projectV1ontoV2(Vector3 v1, Vector3 v2)
    {
        return (v2 / v2.length()) * (v1.dotProduct(v2) / v2.length());
    }
}
