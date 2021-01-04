#include <cstdint>

#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreTrays.h"

#include "game/gamesession.h"

using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    enum ControllerState
    {
        MENU,
        DEBUG,
        SESSION
    };

    const Real mapSize = 50;

    class Controller : public ApplicationContext, public InputListener
    {
    public:
        Controller(ControllerState initState) :
            ApplicationContext("SpaceballsApp"), activeState(initState) {}

        // Rajapinta OGREn renderöintilooppiin, peritty
        // ApplicationContextilta
        //=======================================================//
        bool frameRenderingQueued(const FrameEvent &evt)
        {
            if(gs->gameStarted && !gs->gameEnded)
            {
                GameState gState = gs->getGameState();
                switch(gState)
                {
                    case TURN_TRANSITION:
                        break;

                    case TURN_PLACING_BALL:
                        gs->syncCueballWithPlayer();
                        break;

                    case BALLS_ACTIVE:
                        gs->updatePhysics();
                        if(gs->getActiveCam() == BALL) gs->refreshBallCam();
                        break;
                    
                    default:
                        gs->increaseHitPower();
                        gs->updateAimPoint();
                        break;
                }
            }
            else if(gs->gameEnded)
            {
                if(gameEndDelay == 300)
                {
                    trayMgr_->createLabel(
                            TrayLocation::TL_CENTER, "gameWon", 
                            gs->currentPlayer->getName() + " has won the game!", 400
                    );
                }
                if(gameEndDelay-- == 0)
                {
                    endGameSession();
                }
            }
            trayMgr_->frameRendered(evt);
            gs->player->frameRendered(evt);

            return true;
        }

        // Tapahtumien käsittely, metodit ovat InputListener-luokalta
        // perittyjä ja overridattuja
        //=======================================================//
        bool mouseMoved(const MouseMotionEvent& evt)
        {
            if (trayMgr_->mouseMoved(evt)) return true;
            if (activeState != MENU)
            {
                if (gs->isCamInteractive())
                    gs->player->mouseMoved(evt);
            }
            return true;
        }

        bool mouseReleased(const MouseButtonEvent& evt)
        {
            if (activeState != MENU)
            {
                switch (evt.button)
                {
                    case BUTTON_LEFT:
                        gs->hitBall(gs->getCamPosition());
                        break;
                    case BUTTON_RIGHT:
                        gs->switchCamera(0);
                        break;
                    default:
                        break;
                }
            }
            return true;
        }

        bool mouseWheelRolled(const MouseWheelEvent& evt)
        {
            if (activeState != MENU)
            {
                if (gs->getActiveCam() == BALL)
                    gs->player->mouseWheelRolled(evt);
            }
            return true;
        }

        bool keyPressed(const KeyboardEvent& evt)
        {
            Keycode key = evt.keysym.sym;
            trayMgr_->keyPressed(evt);
            if (activeState == MENU)
            {
                switch (key)
                {
                case SDLK_ESCAPE:
                    getRoot()->queueEndRendering();
                    break;
                case SDLK_RETURN:
                    std::cout << "Starting session.." << std::endl;
                    trayMgr_->showTrays();
                    startGameSession();
                    std::cout << "Session started" << std::endl;
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch (key)
                {
                case SDLK_ESCAPE:
                    getRoot()->queueEndRendering();
                    break;

                default:
                    break;
                }
                if (gs->isCamInteractive())
                {
                    switch(key)
                    {
                        case 8:
                            gs->powerDelta = -0.3;
                            break;
                        case SDLK_ESCAPE:
                            getRoot()->queueEndRendering();
                            break;
                        case SDLK_RETURN:
                            gs->powerDelta = 0.3;
                            break;
                        case SDLK_UP:
                            gs->aimPresses[0] = true;
                            break;
                        case SDLK_DOWN:
                            gs->aimPresses[1] = true;
                            break;
                        case SDLK_LEFT:
                            gs->aimPresses[2] = true;
                            break;
                        case SDLK_RIGHT:
                            gs->aimPresses[3] = true;
                            break;
                    }
                    if(isWASD(key) || key == SDLK_LSHIFT)
                        gs->player->keyPressed(evt);
                }
            }
            return true;
        }

        bool keyReleased(const KeyboardEvent& evt)
        {
            Keycode key = evt.keysym.sym;
            trayMgr_->keyReleased(evt);
            if (activeState != MENU)
            {
                if (evt.keysym.sym == SDLK_SPACE)
                    gs->switchCamera(1);
                else if (evt.keysym.sym == 'r')
                {
                    gs->restart();
                }
                if(gs->isCamInteractive())
                {
                    switch(key)
                    {
                        case 8:
                            gs->powerDelta = 0;
                            break;
                        case SDLK_RETURN:
                            gs->powerDelta = 0;
                            break;
                        case SDLK_UP:
                            gs->aimPresses[0] = false;
                            break;
                        case SDLK_DOWN:
                            gs->aimPresses[1] = false;
                            break;
                        case SDLK_LEFT:
                            gs->aimPresses[2] = false;
                            break;
                        case SDLK_RIGHT:
                            gs->aimPresses[3] = false;
                            break;
                    }

                    if(isWASD(key) || key == SDLK_LSHIFT)
                        gs->player->keyReleased(evt);
                }                    
            }
            if (activeState == DEBUG)
            {
                if (evt.keysym.sym == 'b')
                {
                    std::cout << "Adding ball..." << std::endl;
                    SceneNode* newBallNode = scnMgr_->getRootSceneNode()->createChildSceneNode();
                    newBallNode->setPosition(gs->getCamPosition());
                    Entity* newBallEnt = scnMgr_->createEntity("NUM8.mesh");
                    gs->createBall(newBallNode, newBallEnt, 8);
                }
                else if(evt.keysym.sym == SDLK_SPACE)
                    gs->switchCamera(1);
            }
            return true;
        }

        bool isWASD(Keycode key) { return (key == 'w' || key == 'a' || key == 's' || key == 'd'); }
        //=======================================================//
        
        // Controllerin alustus
        //=======================================================//
        void setup(void)
        {
            ApplicationContext::setup();
            
            addInputListener(this);
         
            Root* root = getRoot();
            RenderWindow* rWin = getRenderWindow();

            // Alustetaan OGREn Scene rajapinta
            scnMgr_ = root->createSceneManager();
            scnMgr_->addRenderQueueListener(getOverlaySystem());
         
            RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
            shadergen->addSceneManager(scnMgr_);

            setWindowGrab();

            // Alustetaan GUIn rajapinta
            trayMgr_.reset(new TrayManager("GUI", rWin));
            trayMgr_->hideCursor();

            addInputListener(trayMgr_.get());
         
            // Luodaan kamera
            cam_ = scnMgr_->createCamera(CAM_0);
            cam_->setNearClipDistance(1);
         
            Viewport* vp = rWin->addViewport(cam_);
            cam_->setAspectRatio((Real)vp->getActualWidth() / (Real)vp->getActualHeight());

            // Alustetaan IrrKlangin äänihallinnoija
            soundMgr_ = createIrrKlangDevice();

            std::string root_path = "";
            root_path.append("C:/Users/jaakko/spaceballs");
            std::string musicfile = root_path + "/sounds/cantina.mp3";
            const char* cstr = musicfile.c_str();

            auto it = soundMgr_->play3D(cstr, vec3df(0, 0, 0), true, false, true);
            if (it) {
                it->setVolume(0.1);
            }

            setupDefaultGameSession();

            showMenu();
        }

        // Muistin vapauttamista
        void cleanup(bool isFinal=false)
        {
            delete gs;
            trayMgr_->destroyAllWidgets();
            scnMgr_->clearScene();

            if(isFinal)
            {
                soundMgr_->drop();
                trayMgr_.reset();
            }
        }

    private:

        // Apumetodi Menu-näkymään siirtymiselle
        void showMenu() 
        { 
            gs->switchCamera(GLOBAL);
            textNode_->setVisible(true); 
        }

        // GameSession olion luonti ja standardipelitilanteen alustaminen
        void setupDefaultGameSession()
        {
            scnMgr_->setAmbientLight(ColourValue(0.25, 0.25, 0.4));
            scnMgr_->setSkyBox(true, "StarrySkybox", 301);

            PhysicsParams gsPhysics
            {
                0.0005, 
                200, 
                0.99, 
                1
            };
            GameParams gsGame
            {
                mapSize, 
                7.1, 
                4,
                activeState == DEBUG
            };

            gs = new GameSession(scnMgr_, trayMgr_.get(), soundMgr_, gsPhysics, gsGame, cam_);

            // Luodaan menun teksti
            Entity* startText = scnMgr_->createEntity("Text.mesh");
            textNode_ = scnMgr_->getRootSceneNode()->createChildSceneNode();
            textNode_->setScale(Vector3(2, 2, 2));
            textNode_->setPosition(gs->getCamPosition());
            textNode_->translate(10*(gs->player->getCamera()->getOrientation().zAxis() * -1));
            textNode_->attachObject(startText);
         
            // Luodaan pelikentän malli
            Vector3 mapScale = Vector3(mapSize, mapSize, mapSize);

            Entity* gameMap = scnMgr_->createEntity("HoleRings.mesh");
            SceneNode* gameMapNode = scnMgr_->getRootSceneNode()->createChildSceneNode();
            gameMapNode->setScale(mapScale);
            gameMapNode->attachObject(gameMap);

            Entity* forceField = scnMgr_->createEntity("ForceField.mesh");
            SceneNode* forceFieldNode = scnMgr_->getRootSceneNode()->createChildSceneNode();
            forceFieldNode->setScale(mapScale);
            forceFieldNode->attachObject(forceField);
        }

        // GameSessionin käynnistys
        void startGameSession() 
        {
            textNode_->setVisible(false);
            activeState = activeState == DEBUG ? DEBUG : SESSION;
            gs->start();
        }

        // GameSessionin lopetus, poistetaan vanha GameSession ja luodaan default GameSession
        void endGameSession()
        {
            gameEndDelay = 300;
            cleanup();

            setupDefaultGameSession();
            showMenu();
            activeState = MENU;
        }

        Camera* cam_;
        SceneNode* textNode_;                   // Menu-tekstin node
        std::unique_ptr<TrayManager> trayMgr_;  // OGREn GUI-hallinnnoija
        SceneManager* scnMgr_;                  // OGREn Scene-hallinnoija, joka vastaa kameroista/valoista/kappaleista
        ISoundEngine* soundMgr_;                // IrrKlangin äänihallinnoija
        GameSession* gs;                        // GameSession objekti, vain yksi olemassa kerralla
        ControllerState activeState;            // Tämänhetkinen kontrollerin tila

        int gameEndDelay = 300;                 // Pelin loppumisen jälkeen odotettava aika ennen kuin siirrytään menuun
    };
}
