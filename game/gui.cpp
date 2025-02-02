#include "gui.h"
#include "iostream"

using namespace Ogre;
using namespace OgreBites;

namespace sb
{
    // GUIn alustus
	void Gui::initGui(TrayManager* trayMgr, double startingPower)
	{
		// Luodaan lyöntivoimamittari
		powerMeter = trayMgr->createProgressBar(TrayLocation::TL_BOTTOMRIGHT, "power", "Hit Power", 200, 60);
		powerMeter->setComment(std::to_string(getPowerPercentage(startingPower)) + "%");
		powerMeter->setProgress(Real(startingPower / maxPower));

		// Luodaan pelaajaikoni
		playerIcon = trayMgr->createLabel(TrayLocation::TL_TOPLEFT, "name", currentPlayer->getName(), 200);

		// Luodaan väliviiva
		separator = trayMgr->createSeparator(TrayLocation::TL_TOPLEFT, "separator", 200);
		ballType = trayMgr->createLabel(TrayLocation::TL_TOPLEFT, "ballType", currentPlayer->getType(), 200);

        // Piilotetaan GUI aluksi
        trayMgr->hideTrays();
	}

    // Lyöntivoimamittarin päivitys
	void Gui::updateHitPower(double newPower)
	{
		powerMeter->setComment(std::to_string(getPowerPercentage(newPower)) + "%");
		powerMeter->setProgress(Real(newPower / maxPower));
	}

	void Gui::updateShowingName() { playerIcon->setCaption(currentPlayer->getName()); }

	void Gui::updateShowingType() { ballType->setCaption(currentPlayer->getType()); }

    // Päivitetään vasemman yläkulman pelaajaindikaattori näyttämään uuden
    // pelaajan tietoja
	void Gui::changePlayer(Player* newPlayer)
	{
		currentPlayer = newPlayer;
		updateShowingName();
		updateShowingType();
	}

    // Apumetodi, joka palauttaa tämänhetkisen voiman suhteen maksimivoimaan prosenttina
    int Gui::getPowerPercentage(double power) { return (int)ceil((power / maxPower)*100); }
}


