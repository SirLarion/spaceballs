#include "Ogre.h"
#include "OgreTrays.h"
#include "player.h"

using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    // Apuluokka, joka hallinoi pelin graafista käyttöliittymää OGREn TrayManagerin kautta
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
        ProgressBar* powerMeter; // Lyöntivoimasmittari
        Player* currentPlayer;   // Tämänhetkinen pelaaja
        Label* ballType;         // Tämänhetkisen pelaajan pallotyyppi
        Label* playerIcon;       // Tämänhetkisen pelaajan nimi

        double maxPower;
        int getPowerPercentage(double power);
    };
}
