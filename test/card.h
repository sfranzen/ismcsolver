/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef CARD_H
#define CARD_H

#include <ostream>

struct Card {
    enum Rank : char {
        Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace
    };
    enum Suit : char {
        Clubs = 0, Diamonds, Hearts, Spades
    };

    Rank rank;
    Suit suit;

    bool operator==(const Card &other) const
    {
        return rank == other.rank && suit == other.suit;
    }

    bool operator!=(const Card &other) const
    {
        return !(*this == other);
    }
};

std::ostream &operator<<(std::ostream &out, Card card)
{
    static const std::string ranks {"xx23456789TJQKA"};
    static const std::string suits {"CDHS"};
    return out << ranks.at(card.rank) << suits.at(card.suit);
}

#endif // CARD_H
