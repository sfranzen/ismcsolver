/*
 * Copyright (C) 2019 Steven Franzen <sfranzen85@gmail.com>
 * This file is subject to the terms of the MIT License; see the LICENSE file in
 * the root directory of this distribution.
 */
#ifndef CARD_H
#define CARD_H

#include <string>
#include <ostream>

struct Card {
    enum Rank : int {
        Two = 0, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace
    };
    enum Suit : int {
        Clubs = 0, Diamonds, Hearts, Spades
    };

    Card() = default;

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

    explicit operator int() const
    {
        return rank + 13 * suit;
    }

    operator std::string() const
    {
        static const std::string ranks {"23456789TJQKA"};
        static const std::string suits {"CDHS"};
        std::string s {ranks.at(rank)};
        return s + suits.at(suit);
    }

    friend std::ostream &operator<<(std::ostream &out, Card card)
    {
        return out << std::string(card);
    }
};

inline bool operator<(const Card &a, const Card &b)
{
    return int(a) < int(b);
}

#endif // CARD_H
