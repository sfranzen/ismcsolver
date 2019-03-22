/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2019  Steven Franzen <sfranzen85@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GAME_H
#define GAME_H

#include "../include/game.h"
#include <valarray>
#include <vector>
#include <ostream>

class Game : public ISMCTS::Game<int>
{
public:
    Game(unsigned side = 3);
    virtual Ptr cloneAndRandomise(unsigned observer) const override;
    virtual unsigned currentPlayer() const override;
    virtual std::vector<int> validMoves() const override;
    virtual void doMove(const int move) override;
    virtual double getResult(unsigned player) const override;
    friend std::ostream &operator<<(std::ostream &out, const Game &g);
private:
    std::valarray<int> m_board;
    std::vector<int> m_moves;
    const unsigned m_side;
    unsigned m_player;
    double m_result;
    bool checkWin(int move) const;
    bool checkSlice(const std::slice &s) const;
};

#endif // GAME_H
