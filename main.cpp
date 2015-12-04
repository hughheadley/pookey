#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <windows.h>

using namespace std;

#define maxPlayers 8 //the maximum number of players which can be in one game
#define inputLayerSize 4 //the number of input variables in the neural network, including a bias input
#define hiddenLayerSize 4 //the number of nodes in the hidden layer of the neural network, including a bias input
#define outputLayerSize 2 //the number of output variables in the neural network
#define maxLayerSize 4 //the largest number of nodes in one layer
#define numberVariables 3 //the number of variables which the neural network bases its decision from. Initially pot, probWin and maximumOpponentBet
#define numberLayers 4 //one layer is added for turning the output layer into a bet
#define familyCount 4
#define familyMembers 8

int sortCards(float cards[7], float suits[7])
{   //sorts cards in descending order and sorts suits accordingly
    int i;
    for(i = 0; i < 7; i ++)
    {
        for(int j = 0; j < 7; j ++)
        {
            if(cards[i] > cards[j])
            {
                float temp = cards[i];
                cards[i] = cards[j];
                cards[j] = temp;
                float temp2 = suits[i];
                suits[i] = suits[j];
                suits[j] = temp2;
            }
        }
    }
    return 0; //the array cards[] is sorted but not returned
}

double checkHighCard(float cards[7])
{   //calculate the hand score for a high card
    double handScore = 1;
    for(int i = 0; i < 5; i ++)
    {
        handScore = handScore + cards[i] * pow(0.01, i + 1);
    }
    return handScore;
}

double checkPair(float cards[7])
{   //check if hand has a pair and return hand score
    double handScore = 0;
    for(int i = 0; i < 6; i ++)
    {
        if(cards[i] == cards[i + 1])
        {
            handScore = 2 + cards[i] / 100;
            i = 6;
        }
    }
    if(handScore != 0){
        for(int j = 0; j < 5; j ++){
            handScore = handScore + (cards[j] / 100) * pow(0.01, j + 1);
        }
    }
    return handScore;
}

double checkTwoPair(float cards[7])
{   //check if hand has two pairs and return hand score
    //cards must be sorted in descending order
    double handScore = 0;
    for(int i = 0; i < 4; i ++)
    {
        if(cards[i] == cards[i + 1])
        {
            for(int j = i + 2; j < 6; j ++)
            {
                if(cards[j] == cards[j + 1])
                {
                    //cards[i] is the highest pair value, cards[j] is the other pair value
                    handScore = 3 + cards[i] / 100 + cards[j] / 10000; //first two decimal places indicate highest pair, next two decimal places indicate other pair
                    for(int k = 0; k < 5; k ++)
                    {
                        if((cards[k] != cards[i]) && (cards[k] != cards[j]))
                        {
                            handScore += cards[k] / 1000000; //fifth and sixth decimal places indicate high card
                            //once twopair has been found set i, j and k to their max so that search for twopair ends
                            i = 4;
                            j = 6;
                            k = 5;
                        }
                    }
                }
            }
        }
    }
    return handScore;
}

double checkThreeOfAKind(float cards[7])
{   //check if hand has three of a kind and return hand score
    //cards must be sorted in descending order
    double handScore = 0;
    for(int i = 0; i < 5; i ++)
    {
        if((cards[i] == cards[i + 1]) && (cards[i] == cards[i + 2]))
        {
            handScore = 4 + cards[i] / 100;
            i = 5; //end search for triplet of cards
        }
    }
    if(handScore != 0)
    {
        for(int j = 0; j < 5; j ++)
        {
            handScore += (cards[j] / 100)*pow(0.01, j + 1); //add top 5 cards to the decimal places of handScore to indicate high cards
        }
    }
    return handScore;
}

double checkStraight(float cards[7])
{   //check if hand has a straight and return hand score
    float handScore = 0;
    float topCard = 0;
    for(int i = 0; i < 3; i ++)
    {
        //when i is 3 or greater there cannot be 4 cards in order which follow cards[i]
        topCard = cards[i];
        for(int j1 = i + 1; j1 < 4; j1 ++)
        {
            if(topCard == (cards[j1] + 1))
            {
                for(int j2 = j1 + 1; j2 < 5; j2 ++)
                {
                    if(topCard == (cards[j2] + 2))
                    {
                        for(int j3 = j2 + 1; j3 < 6; j3 ++)
                        {
                            if(topCard == (cards[j3] + 3))
                            {
                                for(int j4 = j3 + 1; j4 < 7; j4 ++)
                                {
                                    if(topCard == (cards[j4] + 4))
                                    {
                                        handScore = 5 + topCard / 100;
                                        i = 3;
                                        j1 = 4;
                                        j2 = 5;
                                        j3 = 6;
                                        j4 = 7;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return handScore;
}

double checkFlush(float cards[7], float suits[7])
{   //check if hand has a flush and return hand score
    //suits must be sorted in descending order with cards sorted accordingly
    double handScore = 0;
    double temp = 0;
    float clubs = 0, diamonds = 0, hearts = 0, spades = 0;
    float flushSuit = 0;
    int j;
    for(int i = 0; i < 7; i ++)
    {
        if(suits[i] == 1)
        {
            clubs += 1;
        }
        else
        {
            if(suits[i] == 2)
            {
                diamonds += 1;
            }
            else
            {
                if(suits[i] == 3)
                {
                hearts += 1;
                }
                else
                {
                    spades += 1;
                }
            }
        }
    }
    //check what suit  has flush
    if(clubs > 4)
    {
        handScore = 6;
        flushSuit = 1;
    }
    else
    {
        if(diamonds > 4)
        {
            handScore= 6;
            flushSuit = 2;
        }
        else
        {
            if(hearts > 4)
            {
                handScore = 6;
                flushSuit = 3;
            }
            else
            {
                if(spades > 4)
                {
                    handScore = 6;
                    flushSuit = 4;
                }
            }
        }
    }
    if(handScore == 6)
    {
        //add first high card
        for(j = 0; j < 3; j ++)
        {
            if(suits[j] == flushSuit)
            {
                temp = cards[j];
                handScore += temp / 100;
                break;
            }
        }
        //add second high card
        for(j = j + 1; j < 4; j ++)
        {
            if(suits[j] == flushSuit)
            {
                temp = cards[j];
                handScore += temp / 10000;
                break;
            }
        }
        //add third high card
        for(j = j + 1; j < 5; j ++)
        {
            if(suits[j] == flushSuit)
            {
                temp = cards[j];
                handScore += temp / 1000000;
                break;
            }
        }
        //add fourth high card
        for(j = j + 1; j < 6; j ++)
        {
            if(suits[j] == flushSuit)
            {
                temp = cards[j];
                handScore += temp / 100000000;
                break;
            }
        }
        //add fifth high card
        for(j = j + 1; j < 7; j ++)
        {
          if(suits[j] == flushSuit)
          {
            temp = cards[j];
            handScore += temp / 10000000000;
            break;
          }
        }
    }
    return handScore;
}

double checkFullHouse(float cards[7])
{   //check if hand has a full house and return hand score
    double handScore = 0;
	for(int i = 0; i < 5; i ++)
    {
		//check for triples
		if((cards[i] == cards[i + 1]) && (cards[i] == cards[i + 2]))
		{
			//check for doubles higher than triples
			for(int j = 0; j < (i - 1); j ++)
			{
				if((cards[j] == cards[j + 1]))
				{
					handScore = 7 + (cards[i] / 100) + (cards[j] / 10000);
                }
            }
			//check for doubles lower than triples
			for(int j = i + 3; j < 7; j ++)
			{
				if((cards[j] == cards[j + 1]))
				{
					handScore = 7 + (cards[i] / 100) + (cards[j] / 10000);
					i = 5;
                    j = 7;
                }
            }
        }
    }
    return handScore;
}

double checkFourOfAKind(float cards[7])
{   //check for four of a kind and return hand score
    double handScore = 0;
    for(int i = 0; i < 5; i ++)
    {
        if((cards[i] == cards[i + 1]) && (cards[i] == cards[i + 2]) && (cards[i] == cards[i + 3]))
        {
            handScore = 8 + cards[i] / 100;
            i = 5;
        }
    }
    if(handScore != 0)
    {
        for(int j = 0; j < 5; j ++)
        {
            handScore += (cards[j] / 100) * pow(0.01, j + 1);
        }
    }
    return handScore;
}

double checkStraightFlush(float cards[7], float suits[7])
{   //check if hand has a straight flush and return hand score
    float handScore = 0;
    float topCard = 0;
    float topSuit = 0; //the suit of the top card
    for(int i = 0; i < 3; i ++)
    {
        topCard = cards[i];
        topSuit = suits[i];
        for(int j1 = i + 1; j1 < 4; j1 ++)
        {
            if((topCard == (cards[j1] + 1)) && (topSuit == suits[j1]))
            {
                for(int j2 = j1 + 1; j2 < 5; j2 ++)
                {
                    if((topCard == (cards[j2] + 2)) && (topSuit == suits[j2]))
                    {
                        for(int j3 = j2 + 1; j3 < 6; j3 ++)
                        {
                            if((topCard == (cards[j3] + 3)) && (topSuit == suits[j3]))
                            {
                                for(int j4 = j3 + 1; j4 < 7; j4 ++)
                                {
                                    if((topCard == (cards[j4] + 4)) && (topSuit == suits[j4]))
                                    {
                                        handScore = 9 + topCard / 100;
                                        i = 3;
                                        j1 = 4;
                                        j2 = 5;
                                        j3 = 6;
                                        j4 = 7;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return handScore;
}

double getHandScore(float cards[7], float suits[7])
{   //returns the value of the best hand from 7 cards
    sortCards(cards, suits); //put the cards in order of highest to lowest
    double handScore = 0;
    handScore = checkStraightFlush(cards, suits);
    if(handScore == 0) //if handScore didn't change then check for the next best hand
        {
        handScore = checkFourOfAKind(cards);
        if(handScore == 0)
        {
            handScore = checkFullHouse(cards);
            if(handScore == 0)
            {
                handScore = checkFlush(cards, suits);
                if(handScore == 0)
                {
                    handScore = checkStraight(cards);
                    if(handScore == 0)
                    {
                        handScore = checkThreeOfAKind(cards);
                        if(handScore == 0)
                        {
                            handScore = checkTwoPair(cards);
                            if(handScore == 0)
                            {
                                handScore = checkPair(cards);
                                if(handScore == 0)
                                {
                                    handScore = checkHighCard(cards);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return handScore;
}

float dealCard(float existingCards[maxPlayers], float existingSuits[maxPlayers], float newCard[2])
{   //dealCard modifies newCard[2] to enter a new card and suit
    int cardIndex, cardNumber, suitNumber;
    int uniqueness = 0; //0 is not unique card, 1 is unique

    //loop until a unique card is found
    while(!uniqueness)
    {
        cardIndex = rand(); //rand must be used so that a new seed isn't required each call
        cardNumber = (cardIndex % 13) + 2;
        suitNumber = (cardIndex % 4) + 1;
        uniqueness = 1;
        for(int j = 0; j < 52; j ++)
        {
            if(existingCards[j] == '/0')
            {
                j = 52; //if all cards have been checked then end loop
            }
            else if((cardNumber == existingCards[j]) && (suitNumber == existingSuits[j]))
            {
                uniqueness = 0;
                j = 52; //if card is not unique then end search
            }
        }
        if(uniqueness == 1)
        {
            newCard[0] = cardNumber;
            newCard[1] = suitNumber;
        }
    }
    return 0;
}

int countPlayers(int playersKnockedOut[maxPlayers])
{   //countPlayers counts how many players are not knocked out
    int numberPlayers = 0;
    for(int i = 0; i < maxPlayers; i ++)
    {
        if(!playersKnockedOut[i])
        {
            numberPlayers ++;
        }
    }
    return numberPlayers;
}

float winProb(float holeCards[2], float holeSuits[2], float communityCards[5], float communitySuits[5], float playersActive)
{   //winProb calculates the probability that the set of cards will beat all remaining players assuming those players have random hands
    //community cards are 0,0 if not yet dealt
    float probability, prob; //prob is the chance of beating one player, probability is the chance of being all players
    float wins = 0;
    double myHandValue = 0, oppHandValue = 0; //this hand's and the opponent's hand's value
    float attempts = 9000; //attempts should be 9000 to be 95% sure that the sample average is less than 0.5% away from the true average
    float deal[2];
    int commCards; // the number of community cards
    float oppCards[7], oppSuits[7]; //opponent's cards and suits
    float existingCards[9], existingSuits[9]; //limited to 5 community cards, my cards and opponent's cards
    float myCards[7], mySuits[7]; //the cards of the player who's winprob is being calculated (including community cards)
    float commonCards[5], commonSuits[5];
    //put community cards into new array
    for(int i = 0; i < 5; i ++)
    {
        commonCards[i] = communityCards[i];
        commonSuits[i] = communitySuits[i];
    }
    //determine how many community cards there are
    for(commCards = 0; commCards < 5; commCards ++)
    {
        if(commonCards[commCards] == 0)
        {
            break;
        }
    }
    //commcards is the number of community cards
    //fill in existing cards and existing suits
    existingCards[0] = holeCards[0];
    existingSuits[0] = holeSuits[0];
    existingCards[1] = holeCards[1];
    existingSuits[1] = holeSuits[1];

    for(int i = 0; i < commCards; i ++)
    {
        existingCards[i + 2] = commonCards[i];
        existingSuits[i + 2] = commonSuits[i];
        myCards[i + 2] = commonCards[i];
        mySuits[i + 2] = commonSuits[i];
    }

    //loop for number of outcomes explored
    for(int i = 0; i < attempts; i ++)
    {
        //fill in my cards and my suits
        myCards[0] = holeCards[0];
        mySuits[0] = holeSuits[0];
        myCards[1] = holeCards[1];
        mySuits[1] = holeSuits[1];
        //generate remaining community cards
        for(int j = commCards; j < 5; j ++)
        {
            dealCard(existingCards, existingSuits, deal);
            existingCards[j + 2] = deal[0];
            existingSuits[j + 2] = deal[1];
            commonCards[j] = deal[0];
            commonSuits[j] = deal[1];
        }
        //generate opponent's cards
        dealCard(existingCards, existingSuits, deal);
        oppCards[0] = deal[0];
        oppSuits[0] = deal[1];
        existingCards[5] = deal[0];
        existingSuits[5] = deal[1];
        dealCard(existingCards, existingSuits, deal);
        oppCards[1] = deal[0];
        oppSuits[1] = deal[1];
        existingCards[6] = deal[0];
        existingSuits[6] = deal[1];
        //fill in my and opponent's cards
        for(int k = 0; k < 5; k ++){
            myCards[k + 2] = commonCards[k];
            mySuits[k + 2] = commonSuits[k];
            oppCards[k + 2] = commonCards[k];
            oppSuits[k + 2] = commonSuits[k];
        }
        myHandValue = getHandScore(myCards, mySuits);
        oppHandValue = getHandScore(oppCards, oppSuits);

        if(myHandValue > oppHandValue){
            wins ++;
        }
    }
    prob = (wins / attempts); //probability of beating one player
    probability = pow(prob, playersActive - 1); //probability of beating all players
    return probability;
}

int printInfo(int playersKnockedOut[maxPlayers], int startPosition, string playerNames[maxPlayers], int chips[maxPlayers], int aiPlayers[maxPlayers])
{   //print players names, dealer's name, players chips;
    int position;
    int numberPlayers = countPlayers(playersKnockedOut);
    cout << "There are " << numberPlayers << " players" << endl;
    cout << "The players in the game are:" << endl;
    for(int i = startPosition; i < maxPlayers + startPosition; i ++)
    {
        position = i % maxPlayers;
        if(playersKnockedOut[position])
        {
            cout << playerNames[position] << "\t" << chips[position];
            if(i == startPosition)
            {
                cout << " (dealer)";
            }
            if(aiPlayers[position] == 1)
            {
                cout << " (computer)";
            }
            cout << endl;
        }
    }
    return 0;
}

int getInfo(string playerNames[maxPlayers], int aiPlayers[maxPlayers], int chips[maxPlayers])
{   //getInfo modifies playerNames, AIplayers and chips arrays and returns the number of players
    int numberPlayersEntry;
    cout << "How many players are there?" << endl;
    cin >> numberPlayersEntry;
    cout << "Enter the dealer's name" << endl;
    cin >> playerNames[0];
    for(int i = 0; i < numberPlayersEntry; i ++)
    {
        cout << "How many chips does " << playerNames[i] << " have?" << endl;
     	cin >> chips[i];
        cout << "Is " << playerNames[i] << " a computer? (enter 1 for yes, 0 for no)" << endl;
     	cin >> aiPlayers[i];
     	cout << endl;
        if(i + 1 < numberPlayersEntry)
        {
            cout << "Enter the next player's name" << endl;
            cin >> playerNames[i + 1];
        }
    }
    return numberPlayersEntry;
}

int checkDetails(int playersKnockedOut[maxPlayers], int startPosition, string playerNames[maxPlayers], int aiPlayers[maxPlayers], int chips[maxPlayers])
{   //checks if the players' names, order, and chip stacks are correct
    int infoCheck = 0; //when infoCheck is 1 the information stored is correct
    int numberPlayers;
    while(!infoCheck)
    {
        //Before round starts print the information to check if it is correct
        printInfo(playersKnockedOut, startPosition, playerNames, chips, aiPlayers);
        //ask if info is correct, if not request names and chips
        cout << "Is information correct? (enter 1 if correct)" << endl;
        cin >> infoCheck;
        if(infoCheck != 1)
        {
            numberPlayers = getInfo(playerNames, aiPlayers, chips);
        }
    }
    return 0;
}

int deal(int roundNumber, int numberPlayers, int folds[maxPlayers], float playerCards[maxPlayers][2], float playerSuits[maxPlayers][2], float existingCards[5 + (maxPlayers * 2)], float existingSuits[5 + (maxPlayers * 2)], float communityCards[5], float communitySuits[5])
{   //deal hole cards or community cards for this round
    float newCard[2] = {0,0};
    if(roundNumber == 1)
    {   //Deal two cards to each person and update existingcards[21]
        for(int i = 0; i < maxPlayers; i ++)
        {
            if(!folds[i])
            {
                for(int j = 0; j < 2; j ++)
                {
                    dealCard(existingCards, existingSuits, newCard);
                    playerCards[i][j] = newCard[0];
                    playerSuits[i][j] = newCard[1];
                    existingCards[(2 * i) + j] = newCard[0];
                    existingSuits[(2 * i) + j] = newCard[1];
                }
            }
        }
    }
    if(roundNumber == 2)
    {
        for(int i = 0; i < 3; i ++)
        {
            dealCard(existingCards, existingSuits, newCard);
            communityCards[i] = newCard[0];
            communitySuits[i] = newCard[1];
            existingCards[(numberPlayers * 2) + i] = newCard[0];
            existingSuits[(numberPlayers * 2) + i] = newCard[1];
        }
        communityCards[3] = 0;
        communityCards[4] = 0;
        communitySuits[3] = 0;
        communitySuits[4] = 0;
    }
    if(roundNumber == 3)
    {
        dealCard(existingCards, existingSuits, newCard);
        communityCards[3] = newCard[0];
        communitySuits[3] = newCard[1];
        existingCards[(numberPlayers * 2) + 3] = newCard[0];
        existingSuits[(numberPlayers * 2) + 3] = newCard[1];
        communityCards[4] = 0;
        communitySuits[4] = 0;
    }
    if(roundNumber == 4)
    {
        dealCard(existingCards, existingSuits, newCard);
        communityCards[4] = newCard[0];
        communitySuits[4] = newCard[1];
        existingCards[(numberPlayers * 2) + 4] = newCard[0];
        existingSuits[(numberPlayers * 2) + 4] = newCard[1];
    }
    return 0;
}

int requestBet()
{   //requestBet asks the player for their bet
    int newbet;
    string betString;
    int validbet = 0;
    while(!validbet)
    {
        cin >> betString;
        validbet = 1; //assume bet is valid then test this
        istringstream(betString) >> newbet; //istringstream returns 0 if string has no number. This must not be confused with a bet of 0
        if(((newbet == 0) && (betString.at(0) != '0')) || (newbet < 0))
        {
            cout << "Enter a valid bet" << endl;
            validbet = 0;
        }
    }
    return newbet;
}

int getHumanBet(int position, int maxBet, int pot, string playerNames[maxPlayers], int chips[maxPlayers], int bets[maxPlayers], int calls[maxPlayers], int raises[maxPlayers])
{   //getHumanBet asks human player for bet and validates it
    int newBet;
    int betValue = 0; //betValue is the bet after modified for validity, betValue is -1 if invalid
    cout << endl << playerNames[position] << " has " << chips[position] << " chips" << " and has bet "  << bets[position] << " already" << endl;
    cout << "The bet to match is " << maxBet << " for a pot of " << pot << endl;
    int callValue = maxBet - bets[position];
    if(callValue < 0)
    {
        callValue = 0;
    }
    cout << playerNames[position] << ", the call value is " << callValue << ". How much are you betting?" << endl;
    //request bet
    newBet = requestBet();
    betValue = newBet;
    if(newBet == 0)
    {    //check if player has folded or checked
        if(bets[position] == maxBet)
        {
            cout << endl << playerNames[position] << " has checked" << endl;
        }
        else
        {
            cout << endl << playerNames[position] << " has folded" << endl;
        }
    }
    else
    {        //check if player cannot bet full amount
        if(chips[position] + bets[position] < maxBet)
        {
            //check that they have bet as much as they can
            if(newBet < chips[position])
            {
                cout << "You must bet all your money or fold" << endl;
                betValue = -1;
            }
        }
        else
        {
            //check if bet is valid
            if(((newBet + bets[position] >= maxBet) || (newBet == chips[position])) && (newBet <= chips[position]))
            {
                //if valid bet do nothing
            }
            else
            {
                betValue = -1; //bet is invalid
                if((chips[position] > newBet) && (newBet != 0))
                {
                    cout << "That's not enough" << endl;
                }
                if((chips[position] < newBet) && (newBet != 0))
                {
                    cout << "That's more than your chip stack" << endl;
                }
            }
        }
    }
    return betValue;
}

char getSuitLetter(float suitNumber)
{   //getSuitLetter converts a suit's number to a card's suit to display
    char suit = ' ';
    if(suitNumber == 1){
        suit = '\5';
    }
    else if(suitNumber == 2){
        suit = '\4';
    }
    else if(suitNumber == 3){
        suit = '\3';
    }
    else if(suitNumber == 4){
        suit = '\6';
    }
    return suit;
}

char getCardLetter(float cardNumber)
{   //getCardLetter converts trick card numbers into letters
    char card = '0';
    if(cardNumber == 14){
        card = 'A';
    }
    else if(cardNumber == 13){
        card = 'K';
    }
    else if(cardNumber == 12){
        card = 'Q';
    }
    else if(cardNumber == 11){
        card = 'J';
    }
    return card;
}

float getCardNumber()
{   //getCardNumber requests the user to enter a card's number until receiving a valid string and converts this string to a integer card number
    string card;
    int invalidCard = 1;
    float cardNumber;
    while(invalidCard)
    {   //while the card entered is invalid repeat loop
        cin >> card;
        if((card == "A") || (card == "a") || (card == "Ace") || (card == "ace") || (card == "ACE") || (card == "14"))
        {
            cardNumber = 14;
            invalidCard = 0;
        }
        else if((card == "K") || (card == "k") || (card == "King") || (card == "king") || (card == "KING") || (card == "13"))
        {
            cardNumber = 13;
            invalidCard = 0;
        }
        else if((card == "Q") || (card == "q") || (card == "Queen") || (card == "queen") || (card == "QUEEN") || (card == "12"))
        {
            cardNumber = 12;
            invalidCard = 0;
        }
        else if((card == "J") || (card == "j") || (card == "Jack") || ( card == "jack") || (card == "JACK") || (card == "11"))
        {
            cardNumber = 11;
            invalidCard = 0;
        }
        else if(card == "10")
        {
            cardNumber = 10;
            invalidCard = 0;
        }
        else if(card == "9")
        {
            cardNumber = 9;
            invalidCard = 0;
        }
        else if(card == "8")
        {
            cardNumber = 8;
            invalidCard = 0;
        }
        else if(card == "7")
        {
            cardNumber = 7;
            invalidCard = 0;
        }
        else if(card == "6")
        {
            cardNumber = 6;
            invalidCard = 0;
        }
        else if(card == "5")
        {
            cardNumber = 5;
            invalidCard = 0;
        }
        else if(card == "4")
        {
            cardNumber = 4;
            invalidCard = 0;
        }
        else if(card == "3")
        {
            cardNumber = 3;
            invalidCard = 0;
        }
        else if(card == "2")
        {
            cardNumber = 2;
            invalidCard = 0;
        }
        else
        {
            cout << "Enter a valid card number" << endl;
        }
    }
    return cardNumber;
}

float getSuitNumber()
{   //getSuitNumber requests the user to enter a card's suit until receiving a valid string and converts this string to a integer suit number
    string suit;
    int invalidSuit = 1;
    float suitNumber;
    while(invalidSuit)
    {
        cin >> suit;
        if((suit == "1")||(suit == "c")||(suit == "C")||(suit == "club")||(suit == "clubs"))
        {
            suitNumber = 1;
            invalidSuit = 0;
        }
        else if((suit == "2")||(suit == "d")||(suit == "D")||(suit == "diamond")||(suit == "diamonds"))
        {
            suitNumber = 2;
            invalidSuit = 0;
        }
        else if((suit == "3")||(suit == "h")||(suit == "H")||(suit == "heart")||(suit == "hearts"))
        {
            suitNumber = 3;
            invalidSuit = 0;
        }
        else if((suit == "4")||(suit == "s")||(suit == "S")||(suit == "spade")||(suit == "spades"))
        {
            suitNumber = 4;
            invalidSuit = 0;
        }
        else
        {
            cout << "Enter a valid suit" << endl;
        }
    }
    return suitNumber;
}

int sortWinners(double handScores[maxPlayers], int position[maxPlayers])
{   //sorts winners puts the positions of players in order of best to worst hands
    for(int i = 0; i < maxPlayers; i ++)
    {
        for(int j = 0; j < maxPlayers ; j ++)
        {
            if(handScores[i] > handScores[j])
            {
                double temp1 = handScores[i];
                handScores[i] = handScores[j];
                handScores[j] = temp1;
                int temp2 = position[i];
                position[i] = position[j];
                position[j] = temp2;
            }
        }
    }
    return 0;
}

int sortBetters(int bets[maxPlayers], int position[maxPlayers])
{   //sorts betters and puts the positions of betters in order of highest to lowest bets
    for(int i = 0; i < maxPlayers; i ++)
    {
        for(int j = 0; j < maxPlayers ; j ++)
        {
            if(bets[i] > bets[j])
            {
                double temp1 = bets[i];
                bets[i] = bets[j];
                bets[j] = temp1;
                int temp2 = position[i];
                position[i] = position[j];
                position[j] = temp2;
            }
        }
    }
    return 0;
}

int *sortIntArrayByDouble(int arrayToSort[], double sortArrayBy[], int arraySize, int sortAscending)
{   //sort arrayToSort in order of sortArrayBy. Sort in descending order if sortAscending is false
    if(sortAscending)
    {   //sort the array in ascending order
        for(int i = 0; i < arraySize; i ++)
        {
            for(int j = 0; j < arraySize ; j ++)
            {
                if(sortArrayBy[i] < sortArrayBy[j])
                {
                    double temp1 = sortArrayBy[i];
                    sortArrayBy[i] = sortArrayBy[j];
                    sortArrayBy[j] = temp1;
                    int temp2 = arrayToSort[i];
                    arrayToSort[i] = arrayToSort[j];
                    arrayToSort[j] = temp2;
                }
            }
        }
    }
    else{
        //sort the array in descending order
        for(int i = 0; i < arraySize; i ++)
        {
            for(int j = 0; j < arraySize ; j ++)
            {
                if(sortArrayBy[i] > sortArrayBy[j])
                {
                    double temp1 = sortArrayBy[i];
                    sortArrayBy[i] = sortArrayBy[j];
                    sortArrayBy[j] = temp1;
                    int temp2 = arrayToSort[i];
                    arrayToSort[i] = arrayToSort[j];
                    arrayToSort[j] = temp2;
                }
            }
        }
    }
    return arrayToSort;
}

int findWinners(int winnerPositions[maxPlayers], float playerCards[maxPlayers][2], float playerSuits[maxPlayers][2], float communityCards[5], float communitySuits[5], int folds[maxPlayers])
{   //findwinners returns positions of best to worst hands
    //initially winnerPositions are in order of player not hand score
    double handScores[maxPlayers];
    for(int k = 0; k < maxPlayers; k ++)
    {
        winnerPositions[k] = k;
        handScores[k] = 0;
    }
    for(int position = 0; position < maxPlayers; position ++){
        //check if player is folded or knocked out, if not then find their handscore
        if(folds[position] == 0){
            //combine community cards and player�s cards
            float handScoreCards[7];
            float handScoreSuits[7];
            for(int j = 0; j < 5; j++){
                handScoreCards[j] = communityCards[j];
                handScoreSuits[j] = communitySuits[j];
            }
            handScoreCards[5] = playerCards[position][0];
            handScoreSuits[5] = playerSuits[position][0];
            handScoreCards[6] = playerCards[position][1];
            handScoreSuits[6] = playerSuits[position][1];
            //find player�s handscore
            handScores[position] = getHandScore(handScoreCards, handScoreSuits);
        }
	}
	sortWinners(handScores, winnerPositions);
    return 0;
}

int showCards(int position, float playerCards[maxPlayers][2], float playerSuits[maxPlayers][2])
{   //showCards prints the hole cards of a single player
    for(int i = 0; i < 2; i++)
    {
        if(playerCards[position][i] != 0)
        {
            char temp = getSuitLetter(playerSuits[position][i]);
            if(playerCards[position][i] > 10)
            {
                char temp2 = getCardLetter(playerCards[position][i]);
                cout << temp2 << " " << temp << endl;
            }
            else
            {
                int temp2 = playerCards[position][i];
                cout << temp2 << " " << temp << endl;
            }
        }
    }
    //end many lines so that next player can't see previous player's cards
    cout << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl;
    return 0;
}

float winnerChange(float selfCards[2], float selfSuits[2], float communityCards[5], float communitySuits[5], int roundNumber)
{  //winnerChange looks at if my hand beats other hands before and after new community cards, returns how often the winner changes
    int winBefore, winAfter; //0 if they win, 1 if I win
    float changes = 0;
    float attempts = 5000; //made a float so that division can be done without truncation
    float averageChange;
    int commCards; //number of comm cards present
    int commCardsBefore;
    float existingCards[9] = {0,0,0,0,0,0,0,0,0};
    float existingSuits[9] = {0,0,0,0,0,0,0,0,0};
    float oppCards[7] = {0,0,0,0,0,0,0};
    float oppSuits[7] = {0,0,0,0,0,0,0};
    float myCards[7] = {0,0,0,0,0,0,0}; //the cards of the player who's winnerchange is being calculated
    float mySuits[7] = {0,0,0,0,0,0,0};
    float deal[2];
    double myHandValue, oppHandValue;
	//generate random hand
	//generate remaining communitycards for old and new
	//fill existingcards for old and new and my cards
	//compare their old hand with my old hand
	//compare their new hand with my new hand
	//if the winner changes then increase int changes by 1.

    if(roundNumber == 2)
    {
        commCards = 3;
        commCardsBefore = 0;
    }
    else if(roundNumber == 3)
    {
        commCards = 4;
        commCardsBefore = 3;
    }
    else if(roundNumber == 4)
    {
        commCards = 5;
        commCardsBefore = 4;
    }
    for(int j = 0; j < commCards; j ++)
    {
        existingCards[j] = communityCards[j];
        existingSuits[j] = communitySuits[j];
    }
    for(int i = 0; i < attempts; i ++)
    {
        winBefore = 0;
        winAfter = 0;
        //generate remaining community cards
        for(int j = commCards; j < 5; j ++)
        {
            dealCard(existingCards, existingSuits, deal);
            existingCards[j] = deal[0];
            existingSuits[j] = deal[1];
            communityCards[j] = deal[0];
            communitySuits[j] = deal[1];
        }
        //generate opponent's cards
        dealCard(existingCards, existingSuits, deal);
        oppCards[0] = deal[0];
        oppSuits[0] = deal[1];
        existingCards[5] = deal[0];
        existingSuits[5] = deal[1];
        dealCard(existingCards, existingSuits, deal);
        oppCards[1] = deal[0];
        oppSuits[1] = deal[1];
        existingCards[6] = deal[0];
        existingSuits[6] = deal[1];
        //fill in my and opponent's cards
        for(int k = 0; k < 5; k ++)
        {
            myCards[k + 2] = communityCards[k];
            mySuits[k + 2] = communitySuits[k];
            oppCards[k + 2] = communityCards[k];
            oppSuits[k + 2] = communitySuits[k];
        }
        myHandValue = getHandScore(myCards, mySuits);
        oppHandValue = getHandScore(oppCards, oppSuits);
        if(myHandValue > oppHandValue)
        {
            winAfter = 1;
        }
        //replace newest community card(s) with a random card to assess winning beforehand
        for(int j = commCardsBefore; j < commCards; j++)
        {
            existingCards[j] = 0;
            existingSuits[j] = 0;
        }
        for(int j = commCardsBefore; j < commCards; j++)
        {
            myCards[j + 2] = 0;
            mySuits[j + 2] = 0;
            oppCards[j + 2] = 0;
            oppSuits[j + 2] = 0;
        }
        myHandValue = getHandScore(myCards, mySuits);
        oppHandValue = getHandScore(oppCards, oppSuits);
        if(myHandValue > oppHandValue)
        {
            winBefore = 1;
        }
        if((winBefore - winAfter) != 0)
        {
            changes ++;
        }
    }
    averageChange = changes / attempts;
    return averageChange;
}

float winProbChange(float communityCards[5], float communitySuits[5], int roundNumber)
{   //winProbChange finds how much the newest community card changes the strength of hole cards
    float strengthBefore, strengthAfter; //strength is the percent of hands which some hole cards beat. This is before and after the newest card
    float sumAbsChanges = 0; //sum of the absolute change in hand strength
    float sumSqChanges = 0, stDevChanges; //sumsqchanges used to calculate variance of changes
    float attempts = 100; //made a float so that division can be done without truncation
    float averageChange;
    int commCards; //number of community cards present
    int commCardsBefore;
    float existingCards[9] = {0,0,0,0,0,0,0,0,0};
    float existingSuits[9] = {0,0,0,0,0,0,0,0,0};
    float holeCards[2] = {0,0};
    float holeSuits[2] = {0,0};
    float priorCommCards[5] = {0,0,0,0,0};
    float priorCommSuits[5] = {0,0,0,0,0};
    float deal[2];
	//generate hole cards, assess change in winning probability before and after the newest community card

    if(roundNumber == 2)
    {
        commCards = 3;
        commCardsBefore = 0;
    }
    else if(roundNumber == 3)
    {
        commCards = 4;
        commCardsBefore = 3;
    }
    else if(roundNumber == 4)
    {
        commCards = 5;
        commCardsBefore = 4;
    }
    for(int i = 0; i < attempts; i++)
    {
        for(int j = 0; j < commCards; j++)
        {
            existingCards[j] = communityCards[j];
            existingSuits[j] = communitySuits[j];
            if(j < commCardsBefore)
            {
                priorCommCards[j] = communityCards[j];
                priorCommSuits[j] = communitySuits[j];
            }
            else
            {
                priorCommCards[j] = 0;
                priorCommSuits[j] = 0;
            }
            if(j >= commCards)
            {
                communityCards[j] = 0;
                communitySuits[j] = 0;
            }
        }
        //deal hole cards
        for(int j = 0; j < 2; j++)
        {
            dealCard(existingCards, existingSuits, deal);
            holeCards[j] = deal[0];
            holeSuits[j] = deal[1];
            existingCards[commCards] = deal[0];
            existingSuits[commCards] = deal[1];
        }

        //calculate probability of winning before and after
        strengthBefore = winProb(holeCards, holeSuits, priorCommCards, priorCommSuits, 2);
        strengthAfter = winProb(holeCards, holeSuits, communityCards, communitySuits, 2);
        if(strengthAfter > strengthBefore)
        {
            sumAbsChanges = sumAbsChanges + strengthAfter - strengthBefore;
        }
        else
        {
            sumAbsChanges = sumAbsChanges + strengthBefore - strengthAfter;
        }
        sumSqChanges = sumSqChanges + pow((strengthAfter - strengthBefore), 2);
    }
    averageChange = sumAbsChanges / attempts;
    stDevChanges = pow((sumSqChanges / attempts - pow(averageChange, 2)), 0.5);
    cout << "averagechange is " << averageChange << endl;
    cout << "stdevchanges is " << stDevChanges << endl;
    return averageChange;
}

int oneLayerFeedForward(float currentLayerNodes[], int currentLayerSize, float nextLayerNodes[], int nextLayerSize, float weights[maxLayerSize][maxLayerSize], int applySigmoid)
{   //oneLayerFeedForward calculates one layer of the neural network with the sigmoid function applied
    //nextlayernodes is modified to give the output values without an array returned
    //applysigmoid indicates if the sigmoid function is applied to the output

    //fill output array with figures before sigmoid function is applied
    for(int i = 0; i < nextLayerSize; i ++)
    {
        for(int j = 0; j < currentLayerSize; j ++)
        {
            nextLayerNodes[i] += currentLayerNodes[j] * weights[j][i];
        }
    }
    if(applySigmoid == 1)
    {
        //apply sigmoid function
        for(int i = 0; i < nextLayerSize; i ++)
        {
            nextLayerNodes[i] = 1 / (1 + pow(2.718282, -1 * nextLayerNodes[i]));
        }
    }
    return 0;
}

int neuralNetwork(float pot, float handStrength, float callValue, float bigBlind, float weights01[maxLayerSize][maxLayerSize], float weights12[maxLayerSize][maxLayerSize])
{   //NeuralNetwork takes inputs and weights and returns the amount to bet
    float inputLayer[inputLayerSize];
    float hiddenLayer[hiddenLayerSize];
    float outputLayer[outputLayerSize];
    int amountBet;
    inputLayer[0] = pot / bigBlind;
    inputLayer[1] = handStrength;
    inputLayer[2] = 1; //bias input in neural network
    oneLayerFeedForward(inputLayer, inputLayerSize, hiddenLayer, hiddenLayerSize, weights01, 1);
    oneLayerFeedForward(hiddenLayer, hiddenLayerSize, outputLayer, outputLayerSize, weights12, 0);
    //if the first output is less than 0.5 then check/fold
    if(outputLayer[0] < 0)
    {
        amountBet = 0;
    }
    //else if the second output is less than 0 then call/check
    else if(outputLayer[1] < 0)
    {
        amountBet = callValue;
    }
    //else raise
    else
    {
        /*cout << "callvalue is " << callValue << endl;
        cout << "bigBlind is " << bigBlind << endl;
        cout << "outputlayer1 is " << outputLayer[1] << endl;*/
        amountBet = callValue + (outputLayer[1] * bigBlind);
    }

    if(amountBet < 0)
    {
        amountBet = 0;
    }

    return amountBet;
}

int simpleDecision(int position, int callValue, int chips, int pot, int bigBlind, int calls[maxPlayers], int raises[maxPlayers], float holeCards[2], float holeSuits[2], float communityCards[5], float communitySuits[5], int playersActive)
{   //simpleDecision returns a bet based on hand strength only
    float winChance = winProb(holeCards, holeSuits, communityCards, communitySuits, playersActive);
    int newBet;
    if(winChance < 0.1)
    {
        newBet = 0;
    }
    else if(winChance < 0.3)
    {
        newBet = callValue;
    }
    else
    {
        newBet = callValue + pot * 0.1;
    }
    if(newBet > chips)
    {
        newBet = 0;
    }
    return newBet;
}

int decision(int position, int callValue, int chips, int pot, int bigBlind, int calls[maxPlayers], int raises[maxPlayers], float holeCards[2], float holeSuits[2], float communityCards[5], float communitySuits[5], int playersActive, float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize])
{   //decision returns bet which is got using neural network decision method
    float winChance = winProb(holeCards, holeSuits, communityCards, communitySuits, playersActive);
    int newBet;
    float weights01[maxLayerSize][maxLayerSize];
    float weights12[maxLayerSize][maxLayerSize];
    for(int i = 0; i < maxLayerSize; i ++)
    {
        for(int j = 0; j < maxLayerSize; j ++)
        {
            weights01[i][j] = playerWeights[position][0][i][j];
            weights12[i][j] = playerWeights[position][1][i][j];
        }
    }
    newBet = neuralNetwork(pot, winChance, callValue, bigBlind, weights01, weights12);
    if(newBet > chips)
    {
        newBet = chips;
    }

    return newBet;
}

int getBet(int maxBet, int position, int pot, int bigBlind, string playerNames[maxPlayers], int aiPlayers[maxPlayers], int chips[maxPlayers], int bets[maxPlayers], int calls[maxPlayers], int raises[maxPlayers], float holeCards[2], float holeSuits[2], float communityCards[5], float communitySuits[5], int playersActive, float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize])
{   //return the new bet made by either AI or human players
    int newBet = -1;
    int callValue = maxBet - bets[position];
    if(callValue < 0)
    {
        callValue = 0;
    }
    //if it is an AI player then use decision algorithm
    if(aiPlayers[position] == 1)
    {
        //newBet = simpleDecision(position, callValue, chips[position], pot, bigBlind, calls, raises, holeCards, holeSuits, communityCards, communitySuits, playersActive);
        newBet = decision(position, callValue, chips[position], pot, bigBlind, calls, raises, holeCards, holeSuits, communityCards, communitySuits, playersActive, playerWeights);
    }
    else
    {
        while(newBet < 0)
        {
            newBet = getHumanBet(position, maxBet, pot, playerNames, chips, bets, calls, raises);
            if(newBet < 0)
            {
                cout << "Enter a valid bet" << endl;
            }
        }
    }
    return newBet;
}

int *updateValues(int trainingMode, int position, int newBet, int maxBet, int playersActive, int pot, int active[maxPlayers], int chips[maxPlayers], int calls[maxPlayers], int bets[maxPlayers], int raises[maxPlayers], int folds[maxPlayers], string playerNames[maxPlayers])
{   //updateValues takes a valid new bet and the current state of the game and updates the state of the game. Integers values which aren't in an array are returned in updatedValues array "updatedValues"
    int updatedValues[4];

    //if player folds
    if((newBet == 0) && (maxBet < (bets[position] + chips[position])) && (!folds[position]))
    {//conditions for folding: bet 0 and have enough chips to place maxBet and not be folded already
        if(!trainingMode)
        {
            cout << endl  << playerNames[position] << " has folded" << endl;
        }
        playersActive --;
        folds[position] = 1;
    }

    //if player checks
    if((newBet == 0) && (maxBet == bets[position]))
    {
        if(!trainingMode)
        {
            cout << endl  << playerNames[position] << " has checked" << endl;
        }
    }

    //check if player can afford to call
    if((chips[position] + bets[position]) >= maxBet)
    {
        //if player raises or calls
        if(newBet >= (maxBet - bets[position]))
        {
            calls[position] += maxBet - bets[position];
            raises[position] += bets[position] + newBet - maxBet;
            maxBet = bets[position] + newBet;
        }
    }
    else
    {
        //if player goes all in
        if(newBet == chips[position])
        {
            calls[position] += newBet;
        }
    }

    bets[position] += newBet;
    chips[position] -= newBet;
    active[position] = 1;
    pot += newBet;

    //announce bet made
    if(newBet != 0)
    {
        if(!trainingMode)
        {
            cout << endl << playerNames[position] << " has bet " << newBet << ". " << playerNames[position] << " now has " << chips[position] << " chips" << endl;
        }
    }

    updatedValues[0] = playersActive;
    updatedValues[1] = maxBet;
    updatedValues[2] = pot;

    return updatedValues;
}

int *setBlinds(int dealerPosition, int bigBlind, int chips[maxPlayers], int bets[maxPlayers], int calls[maxPlayers], int playersKnockedOut[maxPlayers])
{   //setBlinds finds the position of the small and big blinds and sets their bet. The positions of the blinds, the blind bets, and the pot are returned in an array
    int blindInfo[5];
    int smallBlindPosition, bigBlindPosition;
    //find small blind position
    for(int i = (dealerPosition + 1); i < (dealerPosition + maxPlayers); i ++)
    {   //starting 1 seat after the dealer, search for the position of the small blind player - the first player after the dealer who is not knocked out
        if(!playersKnockedOut[i % maxPlayers])
        {
            smallBlindPosition = i % maxPlayers;
            i = (dealerPosition + maxPlayers); //stop loop
        }
    }

    //find big blind position
    for(int i = (smallBlindPosition + 1); i < (dealerPosition + maxPlayers + 1); i ++)
    {   //starting 1 seat after the small blind, search for the position of the big blind player - the first player not knocked out
        if(!playersKnockedOut[i % maxPlayers])
        {
            bigBlindPosition = i % maxPlayers;
            i = (dealerPosition + maxPlayers + 1);
        }
    }

    //calculate blind bets and update bets, chips & calls
    int smallBlindBet, bigBlindBet;

    //if small blind player cannot afford to bet small blind then go all in
    if(chips[smallBlindPosition] >= (bigBlind / 2))
    {
        smallBlindBet = bigBlind / 2;
    }
    else
    {
        smallBlindBet = chips[smallBlindPosition];
    }
    bets[smallBlindPosition] = smallBlindBet;
    chips[smallBlindPosition] -= smallBlindBet;
    calls[smallBlindPosition] = smallBlindBet;

    //if big blind player cannot afford to bet big blind then go all in
    if(chips[bigBlindPosition] >= bigBlind)
    {
        bigBlindBet = bigBlind;
    }
    else
    {
        bigBlindBet = chips[bigBlindPosition];
    }
    bets[bigBlindPosition] = bigBlindBet; //set small blind
    chips[bigBlindPosition] -= bigBlindBet;
    calls[bigBlindPosition] = bigBlindBet;

    int pot = bigBlindBet + smallBlindBet;

    blindInfo[0] = smallBlindPosition;
    blindInfo[1] = bigBlindPosition;
    blindInfo[2] = pot;

    return blindInfo;
}

int playHand(int dealerPosition, int trainingMode, int aiPlayers[maxPlayers], int chips[maxPlayers], string playerNames[maxPlayers], int playersKnockedOut[maxPlayers], int numberPlayers, int bigBlind, int manualDealing, float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize])
{   //playhand plays one hand of poker and modifies players' chip counts
    //If trainingMode is true then nothing is printed and there is no "enter to continue"
    cout << endl << endl << endl << endl << "playhand begins" << endl << endl << endl << endl ;
    int initialTotalChips = 0;
    for(int i = 0; i < maxPlayers; i++)
    {
        initialTotalChips += chips[i];
    }
    int pot = 0;
    int playersActive = numberPlayers; //playersactive is number of players not folded, numberplayers is number of players not knocked out
    int position;
    int roundActive = 1; //is the round still being played
    int maxBet = bigBlind;
    int newBet;
    bigBlind = 2;
    int roundNumber;
    int bets[maxPlayers] = {0}, calls[maxPlayers] = {0}, raises[maxPlayers] = {0}; //amount a player has bet/called/raised
    int folds[maxPlayers] = {0}; //0 if player hasn't folded, 1 otherwise
    int active[maxPlayers] = {0}; //0 if a player has not acted this betting round, 1 otherwise
    float playerCards[maxPlayers][2];
    float playerSuits[maxPlayers][2];
    float existingCards[5 + (2 * maxPlayers)] = {0};
    float existingSuits[5 + (2 * maxPlayers)] = {0};
    float communityCards[5] = {0}; //value of the cards which every player can use
    float communitySuits[5] = {0}; //value of the suits which every player can use
    float holeCards[2], holeSuits[2]; //cards and suits of a given player

    //do stuff that is needed before round 1 starts
    for(int k = 0; k < maxPlayers; k ++)
    {   //populate various arrays defining each player's situation
        playerCards[k][0] = 0;
        playerCards[k][1] = 0;
        playerSuits[k][0] = 0;
        playerSuits[k][1] = 0;
        if(playersKnockedOut[k])
        {
            folds[k] = 1;
        }
    }

    //set the blinds and starting position
    int *blindInfo = setBlinds(dealerPosition, bigBlind, chips, bets, calls, playersKnockedOut);
    int smallBlindPosition = blindInfo[0];
    int bigBlindPosition = blindInfo[1];
    pot = blindInfo[2];
    position = (bigBlindPosition + 1) % maxPlayers; //betting starts from 1 after the big blind if it is the first round

    //begin play for the four rounds of betting
    for(roundNumber = 1; roundNumber < 5; roundNumber ++)
    {
        //reset activity at the start of a round
        roundActive = 1;
        for(int k = 0; k < maxPlayers; k ++)
        {
            active[k] = 0;
        }

        //round 1-4 begins
        if(manualDealing == 0)
        {
            deal(roundNumber, numberPlayers, folds, playerCards, playerSuits, existingCards, existingSuits, communityCards, communitySuits);
            //tell people their cards
            if(roundNumber == 1)
            {
                for(int k = 0; k < maxPlayers; k ++)
                {
                    if((aiPlayers[k] == 0) && (playersKnockedOut[k] == 0))
                    {
                        if(!trainingMode)
                        {
                            cout << playerNames[k] << " your cards are:" << endl << endl;
                            showCards(k, playerCards, playerSuits);
                            cout << "enter anything to continue" << endl;
                            cout << endl;
                        }
                    }
                }
            }
        }

        if(manualDealing == 1)
        {
            string temp;
            if(roundNumber == 1)
            {
                for(int k = (dealerPosition + 1); k < (maxPlayers + dealerPosition + 1); k ++)
                {   //begin dealing from the dealer's left
                    int dealPosition = k % maxPlayers;
                    if(!playersKnockedOut[position])
                    {
                    cout << "Enter " << playerNames[dealPosition] << "'s first card" << endl;
                    playerCards[dealPosition][0] = getCardNumber();
                    cout << "Enter that card's suit" << endl;
                    playerSuits[dealPosition][0] = getSuitNumber();
                    cout << "Enter " << playerNames[dealPosition] << "'s second card" << endl;
                    playerCards[dealPosition][1] = getCardNumber();
                    cout << "Enter that card's suit" << endl;
                    playerSuits[dealPosition][1] = getSuitNumber();
                    }
                }
            }
            if(roundNumber == 2)
            {
                cout << "Enter the first flop card" << endl;
                communityCards[0] = getCardNumber();
                cout << "Enter that card's suit" << endl;
                communitySuits[0] = getSuitNumber();
                cout << "Enter the second flop card" << endl;
                communityCards[1] = getCardNumber();
                cout << "Enter that card's suit" << endl;
                communitySuits[1] = getSuitNumber();
                cout << "Enter the third flop card" << endl;
                communityCards[2] = getCardNumber();
                cout << "Enter that card's suit" << endl;
                communitySuits[2] = getSuitNumber();
                communityCards[3] = 0;
                communitySuits[3] = 0;
                communityCards[4] = 0;
                communitySuits[4] = 0;
            }
            if(roundNumber == 3)
            {
                cout << "Enter the turn card" << endl;
                communityCards[3] = getCardNumber();
                cout << "Enter that card's suit" << endl;
                communitySuits[3] = getSuitNumber();
                communityCards[4] = 0;
                communitySuits[4] = 0;
            }
            if(roundNumber == 4)
            {
                cout << "Enter the river card" << endl;
                communityCards[4] = getCardNumber();
                cout << "Enter that card's suit" << endl;
                communitySuits[4] = getSuitNumber();
            }
        }

        //print community cards
        if(!trainingMode)
        {
            if(roundNumber != 1)
            {
                cout << endl << "community cards are:" << endl;
                for(int k = 0; k < 5; k ++)
                {
                    if(communityCards[k] != 0)
                    {
                        char temp = getSuitLetter(communitySuits[k]);
                        if(communityCards[k] > 10)
                        {
                            char temp2 = getCardLetter(communityCards[k]);
                            cout << temp2 << " " << temp << endl;
                        }
                        else
                        {
                            int temp2 = communityCards[k];
                            cout << temp2 << " " << temp << endl;
                        }
                    }
                }
            }
        }

        //betting begins
        while(roundActive)
        {
            if(!folds[position] && !trainingMode)
            {//only make a line break when players aren't folded, this avoids inconsistent numbers of line breaks
                cout << endl;
            }
            newBet = 0;
            /*cout << endl;
            for(int i = 0; i < 8; i++)
            {
                cout << "bets[" << i << "] is" << bets[i] << endl;
                cout << "chips[" << i << "] is" << chips[i] << endl;
                cout << "calls[" << i << "] is" << calls[i] << endl;
                cout << "raises[" << i << "] is" << raises[i] << endl;
                cout << "folds[" << i << "] is" << folds[i] << endl;
                cout << "active[" << i << "] is" << active[i] << endl;
            }
            cout << "position to play is" << position << endl;
            cout << "playersActive is" << playersActive << endl;
            cout << "maxBet is" << maxBet << endl;
            cout << "bets is" << bets[position] << endl;
            cout << "smallBlindPosition is" << smallBlindPosition << endl;
            cout << "bigBlindPosition is" << bigBlindPosition << endl;*/
            //check if round is over. Round ends if there is 1 player or if a player who has acted already has nothing to call
            if((playersActive == 1) || (active[position] && (bets[position] == maxBet)))
            {
                roundActive = 0;
            }
            else
            {
                //check if this player has not folded (players are folded by default if knocked out)
                if(!folds[position]){
                    if(!trainingMode && aiPlayers[position])
                    {
                        cout << "Enter anything to continue" << endl;
                        string temp;
                        cin >> temp;
                        cout << endl;
                    }
                    //check if this player cannot bet
                    if(chips[position] == 0)
                    {
                        if(!trainingMode)
                        {
                            cout << playerNames[position] << " cannot bet" << endl;
                        }
                    }
                    else
                    {
                        holeCards[0] = playerCards[position][0];
                        holeCards[1] = playerCards[position][1];
                        holeSuits[0] = playerSuits[position][0];
                        holeSuits[1] = playerSuits[position][1];
                        newBet = getBet(maxBet, position, pot, bigBlind, playerNames, aiPlayers, chips, bets, calls, raises, holeCards, holeSuits, communityCards, communitySuits, playersActive, playerWeights);
                    }
                }
            }
            //update values after bet is made
            int * updatedValues = updateValues(trainingMode, position, newBet, maxBet, playersActive, pot, active, chips, calls, bets, raises, folds, playerNames);
            playersActive = updatedValues[0];
            maxBet = updatedValues[1];
            pot = updatedValues[2];

                    /*    int totalChips = 0;
                        for(int i = 0; i < maxPlayers; i++)
                        {
                            totalChips += chips[i];
                        }
                        int totalBets = 0;
                        for(int i = 0; i < maxPlayers; i++)
                        {
                            totalBets += bets[i];
                        }

                        if((totalChips + totalBets) != initialTotalChips)
                        {
                            cout << "Total Chips and bets do not sum to initial total chips" << endl;
                            cout << "position is " << position << endl;
                            cout << "totalChips is " << totalChips << endl;
                            cout << "totalBets is " << totalBets << endl;
                            cout << "initialtotalChips is " << initialTotalChips << endl;
                            cout << "newBet is " << newBet << endl;
                            cout << "maxBet is " << maxBet << endl;
                            cout << "pot is " << pot << endl;
                        }
                        if(bets[position] != (raises[position] + calls[position]))
                        {
                            cout << "raises and calls don't sum to bets" << endl;
                            cout << "position is " << position << endl;
                            cout << "bets[position] is " << bets[position] << endl;
                            cout << "raises[position] is " << raises[position] << endl;
                            cout << "calls[position] is " << calls[position] << endl;
                            cout << "newBet is " << newBet << endl;
                            cout << "maxBet is " << maxBet << endl;
                            cout << "pot is " << pot << endl;
                        }
                        for(int i = 0; i < maxPlayers; i ++)
                        {
                            cout << "bets[" << i << "] is" << bets[i] << endl;
                            cout << "chips[" << i << "] is" << chips[i] << endl;
                            cout << "calls[" << i << "] is" << calls[i] << endl;
                            cout << "raises[" << i << "] is" << raises[i] << endl;
                            cout << "folds[" << i << "] is" << folds[i] << endl;
                            cout << "active[" << i << "] is" << active[i] << endl;
                        }
                        if( (bets[position] != (raises[position] + calls[position])) ||
                            ((totalChips + totalBets) != initialTotalChips)
                        )
                        {
                            int temp;
                            cin >> temp;
                        }*/

            position = (position + 1) % maxPlayers;
        } //while(roundactive) loop end

        //Find player who bet the most, if nobody matched their bet then reduce it to the second highest bet
        int tempBets[maxPlayers], betPositions[maxPlayers];
        for(int k = 0; k < maxPlayers; k ++)
        {
            tempBets[k] = bets[k];
            betPositions[k] = k;
        }
        sortBetters(tempBets, betPositions);
        if((bets[betPositions[0]] > bets[betPositions[1]]) && (bets[betPositions[0]] > bigBlind))
        {
            chips[betPositions[0]] += (bets[betPositions[0]] - bets[betPositions[1]]);
            raises[betPositions[0]] -= (bets[betPositions[0]] - bets[betPositions[1]]);
            bets[betPositions[0]] = bets[betPositions[1]];
            maxBet = bets[betPositions[1]];
            pot -= (bets[betPositions[0]] - bets[betPositions[1]]);
        }
        if(bets[bigBlindPosition] < bigBlind)
        {
            string temp;    //this line has no use but an error occurs if it is not in place
        }
    }

    //print cards of players who are not folded
    if(!trainingMode)
    {
        for(int i = 0; i < maxPlayers; i ++)
        {
            if(!folds[i])
            {
                holeCards[0] = playerCards[i][0];
                holeCards[1] = playerCards[i][1];
                holeSuits[0] = playerSuits[i][0];
                holeSuits[1] = playerSuits[i][1];
                cout << playerNames[i] << "'s cards are " << endl;
                for(int k = 0; k < 2; k ++)
                {
                    char temp = getSuitLetter(holeSuits[k]);
                    if(holeCards[k] > 10)
                    {
                        char temp2 = getCardLetter(holeCards[k]);
                        cout << temp2 << " " << temp << endl;
                    }
                    else
                    {
                        int temp2 = holeCards[k];
                        cout << temp2 << " " << temp << endl;
                    }
                }
            }
        }
    }

    //find the winner
    int winnerPositions[maxPlayers];
    findWinners(winnerPositions, playerCards, playerSuits, communityCards, communitySuits, folds);
    //give chips to winner(s)
    int sumWinnings; //amount of chips won by a player
    for(int k = 0; k < maxPlayers; k ++)
    {   //loop through all winners
        if(!playersKnockedOut[winnerPositions[k]])
        {
            sumWinnings = 0;
            int winnerBet = bets[winnerPositions[k]];
            if(!trainingMode)
            {
                cout << "The ";
                if(k == 0)
                {
                    cout << "first ";
                }
                else
                {
                    cout << "next ";
                }
                cout << "winner is " << playerNames[winnerPositions[k]] << endl;
            }
            for(int j = 0; j < maxPlayers; j ++)
            {
                //loop through all players to pay winners
                if(winnerBet >= bets[j])
                {
                    sumWinnings += bets[j];
                    bets[j] = 0;
                }
                else
                {
                    sumWinnings += winnerBet;
                    bets[j] -= winnerBet;
                }
            }
            chips[winnerPositions[k]] += sumWinnings;
        }
    }
    for(int k = 0; k < maxPlayers; k ++)
    {
        if(!playersKnockedOut[k] && !trainingMode)
        {
            cout << playerNames[k] << " has a chip stack of " << chips[k] << endl;
        }
    }
    return 0;
}

int *playManyHands(int bigBlind, int manualDealing, int trainingMode, int maxNumberHands, int initialPosition, string playerNames[maxPlayers], int aiPlayers[maxPlayers], int chips[maxPlayers], int playersKnockedOut[maxPlayers])
{   //playManyHands plays a game of poker until maxNumberHands have been played or until there is one player remaining. The players' chips are returned
    //maxNumberHands is the number of hands played before stopping. This is set to 0 if game continues until 1 player is left
    //initialposition is the position of the dealer in the first game
    srand(time(NULL));
    //open file with neural network weights for each player
    fstream myfile ("neuralNetworkWeights.txt");
    float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize];
    int dealerPosition;
    int gameActive = 1;
    int numberPlayers = countPlayers(playersKnockedOut);
    int handsPlayed = 0;
    int k = initialPosition;
    //play until a winner is found
    while(gameActive)
    {
        if(!playersKnockedOut[k % maxPlayers])
        {
            dealerPosition = k % numberPlayers;
            playHand(dealerPosition, trainingMode, aiPlayers, chips, playerNames, playersKnockedOut, numberPlayers, bigBlind, manualDealing, playerWeights);
            handsPlayed ++;
            cout << "number of hands played is " << handsPlayed << endl;
            cout << "number of players is " << numberPlayers << endl;
            if(!trainingMode)
            {
                cout << "enter anything to start next hand \n";
                string temp;
                cin >> temp;
            }
        }
        for(int j = 0; j < maxPlayers; j ++)
        {   //update players knocked out
            if(chips[j] == 0)
            {
                playersKnockedOut[j] = 1;
            }
        }
        numberPlayers = countPlayers(playersKnockedOut);
        if((numberPlayers == 1) || ((handsPlayed == maxNumberHands) && (maxNumberHands != 0)))
        {   //game ends if there is 1 player left or if the maximum number of hands has been played
            gameActive = 0;
        }
        k ++;
    }
    return chips;
}

int selectPlayers(int numberPlayersHand, int gamesPlayed[], int playerRefNumbers[])
{   //selectPlayers modifies the playerRefNumbers array to fill it with those players who are to play the next hand and returns the minimum number of games which all players have played
    //half of the players chosen are those who have played the fewest games, this ensures that eventually all players have played enough games
    //numberPlayersHand is the number of players who are playing together
    //gamesPlayed is an array with the number of times each player has played
    //playerRefNumbers is an array with the reference numbers of each player who is playing this hand, this is modified
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    //find the player who has played the fewest games and add them to playerRefNumber
    int minGames = gamesPlayed[0];
    int minGamesPlayer = 0; //the player with the fewest games played (initial guess: the player with Ref number 0
    for(int i = 1; i < (familyCount * familyMembers); i ++)
    {
        if(gamesPlayed[i] < minGames)
        {
            minGames = gamesPlayed[i];
            minGamesPlayer = i;
        }
    }
    playerRefNumbers[0] = minGamesPlayer;

    //select the remainder of the players to play in this hand at random
    Sleep(1);  //pause for 1 millisecond between function calls to ensure time seed is unique
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> uniformRefs(0, (familyCount * familyMembers) - 1);
    for(int i = 1; i < numberPlayersHand; i ++)
    {//pick a player at random, if they have not already been chosen then add them to the list of playerNumbers
        int refNumber = uniformRefs(generator);
        int uniqueRefNumber = 1;
        for(int j = 0; j < numberPlayersHand; j ++)
        {
            if(refNumber == playerRefNumbers[j])
            {//if true the reference number was not unique and a new one must be generated
                uniqueRefNumber = 0;
                j = numberPlayersHand;
                i --; //reduce i to repeat the selection for this player
            }
        }
        if(uniqueRefNumber == 1)
        {//if unique add the ref number to the list
            playerRefNumbers[i] = refNumber;
        }
    }

    //update games played by each player
    for(int i = 0; i < numberPlayersHand; i ++)
    {
        gamesPlayed[playerRefNumbers[i]] ++;
    }
    return minGames;
}

int generateChips(int bigBlind, float minChips, float maxChips, int numberPlayers, int chipStacks[maxPlayers])
{   //generateChips creates a log-uniform distributed variable between (minChips * bigBlind) and (maxChips * bigBlind)
    //minChips and maxChips are per-big blind
    int chipValue;
    double randomNumber, logRandomNumber;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    Sleep(1);  //pause for 1 millisecond between chip generations to ensure time seed is unique
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<double> uniformdistribution(0.0,1.0);
    for(int i = 0; i < numberPlayers; i ++)
    {
        randomNumber = uniformdistribution(generator);
        logRandomNumber = log(minChips) + (randomNumber * (log(maxChips) - log(minChips)));
        chipValue = bigBlind * pow(2.71828, logRandomNumber);
        chipStacks[i] = chipValue;
    }

    //for those players not playing set their chipstack to 0
    for(int i = numberPlayers; i < maxPlayers; i ++)
    {
        chipStacks[i] = 0;
    }

    return 0;
}

int setToZero(int gamesPlayed[], float gameStats[familyCount * familyMembers][3])
{   //setToZero makes gamesPlayed array and gameStats matrix initially full of zeroes
    for(int i = 0; i < (familyCount * familyMembers); i ++)
    {
        for(int j = 0; j < 2; j ++)
        {
            gameStats[i][j] = 0;
        }
            gamesPlayed[i] = 0;
    }
    return 0;
}

int createGeneFiles(int layerSizes[numberLayers], double minVariableValue, double maxVariableValue)
{   //if learning from scratch create files and fill with random numbers
    ///For each family and player create their file and initialise the NN weights
    ///For each player in the hand open their file and import the NN weights into a matrix playerWeights
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    Sleep(1);  //pause for 1 millisecond between function calls to ensure time seed is unique
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<double> uniformdistribution(minVariableValue, maxVariableValue);
    for(int i = 0; i < familyCount; i ++)
    {
        for(int j = 0; j < familyMembers; j ++)
        {   //create a string for the file name indicating the gene's family number
            stringstream ss1;
            ss1 << i;
            string familyString = ss1.str();
            stringstream ss2;
            ss2 << j;
            string memberString = ss2.str();
            string playerFileName = "family";
            playerFileName.append(familyString);
            playerFileName.append("member");
            playerFileName.append(memberString);
            playerFileName.append(".txt");
            //fstream playerCoefficientsFile;
            //PlayerCoefficientsFile.open("test.txt",fstream::out);
            ofstream playerWeightsFile( playerFileName.c_str() );
            for(int j = 0; j < (numberLayers - 1); j ++)
            {
                for(int k = 0; k < layerSizes[j]; k ++)
                {
                    for(int l = 0; l < layerSizes[j+1]; l ++)
                    {
                        playerWeightsFile << uniformdistribution (generator) << "\t";
                    }
                    playerWeightsFile << "\n";
                }
            }
        }
    }
    return 0;
}

int *setUpGame(int bigBlind, int maxChips, int minChips, int layerSizes[numberLayers], int playerRefNumbers[maxPlayers], int gamesPlayed[familyCount * familyMembers], int chips[maxPlayers], int aiPlayers[maxPlayers], string playerNames[maxPlayers], int playersKnockedOut[maxPlayers], float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize])
{   //setUpGame chooses the players who will play the next game and assigned them an amount of chips. A dealer is also selected.
    int gameInfo[3]; //gameInfo stores the number of players, the dealer's position, and the minimum number of games which players have played
    int dealerPosition;
    int minGamesPlayed;
    //create random number generator for the number of players playing
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    Sleep(1);  //pause for 1 millisecond between function calls to ensure time seed is unique
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> uniformPlayers(2, maxPlayers);

    //choose how many players will play
    int numberPlayersHand = uniformPlayers(generator);

    //select the players who will be playing, fill the refNumbers array with those players
    minGamesPlayed = selectPlayers(numberPlayersHand, gamesPlayed, playerRefNumbers);

    //select how many chips each player will have, fill chips array
    generateChips(bigBlind, minChips, maxChips, numberPlayersHand, chips);

    //create random number generator for selecting which player is dealer
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    Sleep(1);  //pause for 1 millisecond between function calls to ensure time seed is unique
    std::default_random_engine generator2 (seed);
    std::uniform_int_distribution<int> uniformDealer(0, numberPlayersHand - 1);

    //select a dealer
    dealerPosition = uniformDealer (generator2);

    int memberNumber;
    int familyNumber;

    //add the players' variables to the weights array
    for(int playerCount = 0; playerCount < numberPlayersHand; playerCount ++)
    {
        //open the gene's file to populate the playerWeights matrix
        //calculate which member of which family the gene is from
        memberNumber = playerRefNumbers[playerCount] % familyMembers;
        familyNumber = playerRefNumbers[playerCount] / familyMembers;
        stringstream ss1;
        ss1 << familyNumber;
        string familyString = ss1.str();
        stringstream ss2;
        ss2 << memberNumber;
        string memberString = ss2.str();
        string playerFileName = "family";
        playerFileName.append(familyString);
        playerFileName.append("member");
        playerFileName.append(memberString);
        playerFileName.append(".txt");
        //fstream playerCoefficientsFile;
        //PlayerCoefficientsFile.open("test.txt",fstream::out);
        fstream playerWeightsFile( playerFileName.c_str() );
        for(int j = 0; j < (numberLayers - 1); j ++)
        {
            for(int k = 0; k < layerSizes[j]; k ++)
            {
                for(int l = 0; l < layerSizes[j + 1]; l ++)
                {
                    playerWeightsFile >> playerWeights[playerCount][j][k][l];
                }
            }
        }
    }

    //fill arrays required for playing one hand
    for(int i = 0; i < maxPlayers; i ++)
    {
        aiPlayers[i] = 1;
        playerNames[i] = " ";
        if(i < numberPlayersHand)
        {
            playersKnockedOut[i] = 0;
        }
        else
        {
            playersKnockedOut[i] = 1;
        }
    }

    /*for(int i = 0; i < maxPlayers; i ++)
    {
        cout << "Player " << i << " chip stack is " << chips[i] << endl;
    }*/

    gameInfo[0] = dealerPosition;
    gameInfo[1] = numberPlayersHand;
    gameInfo[2] = minGamesPlayed;
    return gameInfo;
}

int *testGeneFitness(int minTrials, int bigBlind, float minChips, float maxChips, int layerSizes[numberLayers])
{
    int chips[maxPlayers];
    int aiPlayers[maxPlayers];
    string playerNames[maxPlayers];
    int playersKnockedOut[maxPlayers];
    int numberPlayersHand, dealerPosition;
    float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize];
    int gamesPlayed[familyCount * familyMembers];
    float geneStats[familyCount * familyMembers][3]; //geneStats contains the sum of profit/loss, sum of square of profit/loss, and variance of profit
    double Zscores[familyCount * familyMembers];
    float profitProbability[familyCount * familyMembers];
    int chipsBefore[maxPlayers];
    int playerRefNumbers[maxPlayers]; //the reference number of each player who play a given game
    int minGamesPlayed = 0; //initially no players have played, minGamesPlayed increases in the while loop

    //before each generation set players' geneStats to zero
    setToZero(gamesPlayed, geneStats);
    while(minGamesPlayed < minTrials)
    {
        int *gameInfo = setUpGame(bigBlind, maxChips, minChips, layerSizes, playerRefNumbers, gamesPlayed, chips, aiPlayers, playerNames, playersKnockedOut, playerWeights);
        dealerPosition = gameInfo[0];
        numberPlayersHand = gameInfo[1];
        minGamesPlayed = gameInfo[2];

        cout << endl << endl << "ref numbers" << endl;
        for(int i = 0; i < numberPlayersHand; i ++)
        {
            cout << "Player " << i << " refNumber " << playerRefNumbers[i] << " has chip stack " << chips[i]  << endl;
        }

        for(int i = 0; i < numberPlayersHand; i ++)
        {
            //record players' chips before the game
            chipsBefore[i] = chips[i];
        }

        playHand(dealerPosition, 1, aiPlayers, chips, playerNames, playersKnockedOut, numberPlayersHand, bigBlind, 0, playerWeights);

        //calculate the profit of each player
        int playerProfit[maxPlayers];
        for(int i = 0; i < numberPlayersHand; i ++)
        {
            //calculate profit
            playerProfit[i] = chips[i] - chipsBefore[i];
        }

        /*cout << endl << endl << "Profit" << endl;
        for(int i = 0; i < numberPlayersHand; i ++)
        {
            cout << "Player " << playerRefNumbers[i] << " Profit is " << playerProfit[i]  << endl;
        }*/

        //update game stats for each gene
        for(int i = 0; i < numberPlayersHand; i ++)
        {
            geneStats[playerRefNumbers[i]][0] += playerProfit[i];
            geneStats[playerRefNumbers[i]][1] += pow(playerProfit[i],2);
        }
    }
    //calculate Z scores for each player
    for(int i = 0; i < (familyCount * familyMembers); i ++)
    {
        geneStats[i][2] = (geneStats[i][1] / gamesPlayed[i]) - pow((geneStats[i][0] / gamesPlayed[i]), 2); //calculate the variance of each gene's profit
        Zscores[i] = (geneStats[i][0] / gamesPlayed[i]) / pow(geneStats[i][2] / gamesPlayed[i], 0.5);
    }

    for(int i = 0; i < (familyCount * familyMembers); i++)
    {
        cout << endl;
        cout << "Player ref " << i << endl;
        cout << "Sumprofit is " << geneStats[i][0] << endl;
        cout << "Sumsqprofit is " << geneStats[i][1] << endl;
        cout << "variance is " << geneStats[i][2] << endl;
        cout << "zScores is " << Zscores[i] << endl;
        cout << "sample size is " << gamesPlayed[i] << endl;
    }
    //create geneRanking, the gene reference numbers sorted by their Z scores
    int *geneRanking = sortIntArrayByDouble(playerRefNumbers, Zscores, maxPlayers, 0);
    return geneRanking;
}

int main()
{
    srand(time(NULL)); //seed srand for the deal() function
    int learnFromScratch = 0; //if learnFromScratch is 1 the files containing gene weights are assumed to be empty. If 0 then exiting genetic information in files is used
    int minNumberTrials = 200; //the minimum number of hands each gene must play to estimate their performance
    double minVariableValue = -1.0, maxVariableValue = 1.0; //the range of values for which weights in the neural network can take
    float minChips = 10, maxChips = 200; //the range of chips (relative to big blind) which players can have in a game
    float bigBlind = 2;
    int layerSizes[numberLayers] = {inputLayerSize, hiddenLayerSize, outputLayerSize, 1};

    //if the algorithm is learning from scratch create the files storing player information
    if(learnFromScratch == 1)
    {
        createGeneFiles(layerSizes, minVariableValue, maxVariableValue);
    }
    testGeneFitness(minNumberTrials, bigBlind, minChips, maxChips, layerSizes);

/*
    int numberPlayers = 2;
    int dealerPosition = 0;
    int manualDealing = 1;
    string playerNames[maxPlayers] = {"Hugh", "nick", "","","","","",""};
    int aiPlayers[maxPlayers] = {0,0,0,0,0,0,0,0};
    int chips[maxPlayers] = {10,20,0,0,0,0,0,0};
    int playersKnockedOut[maxPlayers] = {0,0,1,1,1,1,1,1};
    float playerWeights[maxPlayers][numberLayers][maxLayerSize][maxLayerSize];

    playHand(dealerPosition, 0, aiPlayers, chips, playerNames, playersKnockedOut, numberPlayers, bigBlind, manualDealing, playerWeights);
    */

    return 0;
}