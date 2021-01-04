#include "gui.h"
#include "iostream"

using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    // GUIn alustus
	void Gui::initGui(TrayManager* trayMgr, double startingPower)
	{
		// Luodaan lyˆntivoimamittari
		powerMeter = trayMgr->createProgressBar(TrayLocation::TL_BOTTOMRIGHT, "power", "Hit Power", 200, 60);
		powerMeter->setComment(std::to_string(getPowerPercentage(startingPower)) + "%");
		powerMeter->setProgress(Real(startingPower / maxPower));

		// Luodaan pelaajaikoni
		playerIcon = trayMgr->createLabel(TrayLocation::TL_TOPLEFT, "name", currentPlayer->getName(), 200);

		// Luodaan v‰liviiva
		separator = trayMgr->createSeparator(TrayLocation::TL_TOPLEFT, "separator", 200);
		ballType = trayMgr->createLabel(TrayLocation::TL_TOPLEFT, "ballType", currentPlayer->getType(), 200);

        // Piilotetaan GUI aluksi
        trayMgr->hideTrays();
	}

    // Lyˆntivoimamittarin p‰ivitys
	void Gui::updateHitPower(double newPower)
	{
		powerMeter->setComment(std::to_string(getPowerPercentage(newPower)) + "%");
		powerMeter->setProgress(Real(newPower / maxPower));
	}

	void Gui::updateShowingName() { playerIcon->setCaption(currentPlayer->getName()); }

	void Gui::updateShowingType() { ballType->setCaption(currentPlayer->getType()); }

    // P‰ivitet‰‰n vasemman yl‰kulman pelaajaindikaattori n‰ytt‰m‰‰n uuden
    // pelaajan tietoja
	void Gui::changePlayer(Player* newPlayer)
	{
		currentPlayer = newPlayer;
		updateShowingName();
		updateShowingType();
	}

    // Apumetodi, joka palauttaa t‰m‰nhetkisen voiman suhteen maksimivoimaan prosenttina
    int Gui::getPowerPercentage(double power) { return (int)ceil((power / maxPower)*100); }
}


