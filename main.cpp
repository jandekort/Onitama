#include <iostream>

#include "Node.h"
#include "NodeVisitor.h"
#include "PieceVisitor.h"
#include "Animal.h"
#include "Cards.h"


int main() {

    Cards c;
    c.addCard(Animal::Dragon);
    c.addCard(Animal::Rabbit);
    c.addCard(Animal::Tiger);
    c.addCard(Animal::Elephant);
    c.addCard(Animal::Cobra);
    c.show();

    c.play(Cards::CardA1);
    c.show();

    auto node = std::make_shared<Node>();

    for(int i=0; i<5; i++) {
        node->addPiece(Piece(Position(i, 0), Piece::Red, i == 2));
        node->addPiece(Piece(Position(i, 4), Piece::Blue, i == 2));
    }

    std::cout << (*node) ;

    Move m(1, 1);
    Piece* p;
    p = &(*node->m_piece).at(2);
    p->accept(m);

    Score s;
    node->accept(s);
    std::cout << "Score = " << node->m_score << std::endl;

    std::cout << (*node) ;

    auto m2 = c.getMove(Cards::CardA2, 1); //
    p->accept(m2);

    std::cout << (*node);

    std::cout << "Size of node: " << sizeof(*node) << std::endl;
    std::cout << "Size of piece: " << sizeof(p) << std::endl;
    std::cout << "Done" << std::endl;
    return 0;
}