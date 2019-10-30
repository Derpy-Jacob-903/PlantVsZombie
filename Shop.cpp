#include "Shop.h"
#include "System.h"
#include "plants/PeaShooter.h"
#include <iostream>
#include <iomanip>
#include <conio.h>
using namespace std;

Shop::Shop(System& sys):
	system(sys),yard(sys.yard),terminal(Terminal::getInstance()),
	factories{
		new PlantFactory<PeaShooter>(sys,10,100,"豌豆射手"),
	}
{
	initUI();
}

void Shop::buy()
{
	terminal->setCursorVisible(true);
	plantChanged(0);
	int curIndex = 0;
	while (true) {
		if (_kbhit()) {
			if (keyPressed(terminal->getKey(), curIndex))
				break;
		}
	}
	terminal->setCursorVisible(false);
	terminal->clearStatus();
}

Shop::~Shop()
{
	for (int i = 0; i < N; i++)
		delete factories[i];
}

void Shop::updateUI()
{
	terminal->locateToScorePos();
	cout << system.getScore();
	terminal->updateSunray(system.getSunray());
	//更新每种植物的状态
	for (int i = 0; i < N; i++) {
		AbstractPlantFactory* apf = factories[i];
		terminal->locateToPlant(i);
		cout << setw(LengthPerPlant) << ' ';
		terminal->locateToPlant(i);
		cout << apf->getName();
		if (apf->available())
			cout << ' ' << apf->getCost();
		else
			cout << " (" << apf->getRate() << "%)";
	}
}

void Shop::initUI()
{
	int totalWidth = COLS * Block::PIXES_PER_COL;
	cout << endl;//第一行留给时间戳
	for (int i = 0; i < totalWidth; i++)
		cout << '=';
	cout << endl;
	cout << "[商店]";
	//得分15列，分界线1列，阳光总数20列
	for (int i = 0; i < totalWidth - 36; i++)
		cout << ' ';
	cout << "阳光总数: ";
	terminal->markSunrayPos();
	cout << system.getSunray();
	terminal->setSplitCol(totalWidth - 16);
	terminal->locateToCol(terminal->getSplitCol());
	cout << '|';
	cout << setw(16) << setiosflags(ios::left) << "   [得分]" << endl;
	for (int i = 0; i < totalWidth; i++)
		cout << '-';
	cout << endl;
	terminal->setScorePos(terminal->getCurrentRow(), totalWidth - 9);
	int rows = N / PlantsPerRow + int(N % PlantsPerRow != 0);
	for (int row = 0; row < rows; row++) {
		for (int i = 0; i < PlantsPerRow; i++) {
			int t = i + row * PlantsPerRow;
			if (t >= N)
				break;
			terminal->locateToPlant(t);
			cout << setw(LengthPerPlant) << ' ';
			terminal->locateToPlant(t);
			cout << factories[t]->getName();
			if (factories[t]->available())
				cout << ' ' << factories[t]->getCost();
			else
				cout << factories[t]->getStatus();
		}
		terminal->locateToCol(terminal->getSplitCol());
		cout << '|';
	}
	terminal->locateToPixelRow(ROWS * Block::PIXES_PER_ROW + rows + 5);
	for (int i = 0; i < totalWidth; i++)
		cout << '=';
	terminal->locateToScorePos();
	cout << system.getScore();
}

void Shop::plantChanged(int i)
{
	terminal->showStatus(string("按方向键选择，回车键确认。当前：") +
		factories[i]->getName());
	terminal->locateToPlant(i);
}

bool Shop::keyPressed(ControlKey key, int& curIndex)
{
	switch (key) {
	case KeyDown: {
		curIndex += PlantsPerRow;
		if (curIndex >= N)
			curIndex = N - 1;
		plantChanged(curIndex);
		break;
	}
	case KeyUp: {
		if (curIndex >= PlantsPerRow) {
			curIndex -= PlantsPerRow;
			plantChanged(curIndex);
		}
		break;
	}
	case KeyLeft: {
		if (curIndex % PlantsPerRow != 0) {
			curIndex -= 1;
			plantChanged(curIndex);
		}
		break;
	}
	case KeyRight: {
		if (curIndex % PlantsPerRow != PlantsPerRow - 1) {
			curIndex += 1;
			if (curIndex >= N)
				curIndex = N - 1;
			plantChanged(curIndex);
		}
		break;
	}
	case KeyEnter: {
		AbstractPlantFactory* apf = factories[curIndex];
		return (addNewPlant(apf));
	}
	case GiveUp:return true;
	}
	return false;
}

bool Shop::addNewPlant(AbstractPlantFactory* factory)
{
	if (!factory->available()) {
		terminal->showStatus(factory->getName() + "尚未装载完成! ");
		return false;
	}
	else if (factory->getCost() > system.getSunray()) {
		terminal->showStatus("对不起，阳光不足！");
		return false;
	}
	terminal->showStatus(
		string("选中") + factory->getName() + ",方向键选择，x取消，ENTER确认");
	int curRow = 0, curCol = 0;
	terminal->locateToBlock(curRow, curCol);
	while (true) {
		ControlKey key = terminal->getKey();
		switch (key)
		{
		case KeyUp: {
			if (curRow > 0) {
				curRow--;
				terminal->locateToBlock(curRow, curCol);
			}
		}
			break;
		case KeyDown: {
			if (curRow < ROWS - 1) {
				curRow++;
				terminal->locateToBlock(curRow, curCol);
			}
		}
			break;
		case KeyLeft: {
			if (curCol > 0) {
				curCol--;
				terminal->locateToBlock(curRow, curCol);
			}
		}
			break;
		case KeyRight:
			if (curCol < COLS - 1) {
				curCol++;
				terminal->locateToBlock(curRow, curCol);
			}
			break;
		case KeyEnter: {
			Block* block = yard.blockAt(curRow, curCol);
			Plant* p = block->currentPlant();
			if (p != nullptr) {
				terminal->showStatus("该位置已有植物，不可种植！");
				break;
			}
			p = (Plant*)factory->newInstance();
			system.useSunray(factory->getCost());
			p->setBlock(curRow, curCol);
			p->place();
			return true;
		}
			break;
		case GiveUp:
			return false;
		}
	}
}
