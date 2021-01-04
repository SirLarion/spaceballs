#include "Ogre.h"
#include "OgreTrays.h"
#include "player.h"

using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    // Apuluokka, joka hallinoi pelin graafista k�ytt�liittym�� OGREn TrayManagerin kautta
    class Gui
    {
    public:
        Gui(Player* startingPlayer) : currentPlayer(startingPlayer), maxPower(40.0) {}

        void initGui(TrayManager* trayMgr, double startingPower);
        void updateHitPower(double newPower);
        void changePlayer(Player* newPlayer);
        void updateShowingName();
        void updateShowingType();

    private:

        Separator* separator;
        ProgressBar* powerMeter; // Ly�ntivoimasmittari
        Player* currentPlayer;   // T�m�nhetkinen pelaaja
        Label* ballType;         // T�m�nhetkisen pelaajan pallotyyppi
        Label* playerIcon;       // T�m�nhetkisen pelaajan nimi

        double maxPower;
        int getPowerPercentage(double power);
    };
}
