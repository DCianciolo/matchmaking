#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iostream>

//This function compares the rank of two players
bool rankCompare(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2) {
    std::string tempFirst = vec1.at(0);
    std::string tempSecond = vec2.at(0);
    return stoi(tempFirst) < stoi(tempSecond);
}

//This function sorts the players by rank
std::vector<std::vector<std::string>> sortPlayers(std::vector<std::vector<std::string>> players) {
    auto tempPlayers = players;
    std::sort(tempPlayers.begin(), tempPlayers.end(), rankCompare);
    return tempPlayers;
}

//Gets the index of a player in the playes vector
int getPlayerIndex(std::vector<std::vector<std::string>> playersVec, std::string player) {
    int index = 0;
    for(std::vector<std::string> playerVec : playersVec) {
        if(playerVec.at(1) == player) {
            return index;
        }
        index++;
    }
    return -1;
}

int main() {
    //Input your database file and any previous pairing files here. MUST BE .TSV, NOT .CSV
    std::string rankFile = "S11 - League Database - Resume.tsv";
    std::vector<std::string>  previousPairings = {"S11 - League Database - W1Pairings.tsv", "S11 - League Database - W2Pairings.tsv"};
    //Setting up variables
    std::vector<std::vector<std::string>> unrankedPlayers;
    std::fstream playerStream;
    playerStream.open(rankFile, std::fstream::in);
    std::string line, word;
    std::vector<std::string> row;
    std::vector<std::string> toAdd = {};
    std::vector<std::vector<std::string>> oldPairings;
    int num = 28;
    int numToAdd = 2;

    //Changes numbers to make sure the proper rankings are retrieved
    if(previousPairings.size() > 0) {
        num = 29;
        numToAdd = 1;
    }
    //std::cout << "0" << std::endl;
    //Reads in the spreadsheet
    int linenum = 0;
    while(getline(playerStream, line)) {
        if(linenum != 0) {
            row.clear();
            toAdd.clear();
            std::stringstream s(line);
            while(getline(s, word, '\t')) {
                row.push_back(word);
            }
            if(row.at(num + 3 * previousPairings.size() + numToAdd) == "TRUE") {
                if(previousPairings.size() > 0) {
                    toAdd.push_back(row.at(num + 3 * previousPairings.size()));
                }
                else {
                    toAdd.push_back(row.at(42).substr(3, 6));
                }
                toAdd.push_back(row.at(27));
                unrankedPlayers.push_back(toAdd);
            }
        }
        linenum++;
    }
    //std::cout << "1" << std::endl;
    //Sorts players by rank, if they aren't already
    std::vector<std::vector<std::string>> rankedPlayers = sortPlayers(unrankedPlayers);

    //Sets up the old pairings vector
    int numPlayers = rankedPlayers.size();
    for(int i = 0; i < numPlayers; i++) {
        oldPairings.push_back({});
    }
    //Adds in all previous pairings, by adding names into the oldpairings vector at whatever index that the player's rank is in
    for(std::string sheet : previousPairings) {
        std::fstream pairingStream;
        pairingStream.open(sheet, std::fstream::in);
        while(getline(pairingStream, line)) {
            row.clear();
            std::stringstream s(line);
            while(getline(s, word, '\t')) {
                row.push_back(word);
            }
            int playerIndex = getPlayerIndex(rankedPlayers, row.at(0));
            if(playerIndex != -1) {
                for(int j = 1; j < 4; j++) {
                    oldPairings.at(playerIndex).push_back(row.at(j));
                }
            }
        }
    }

    //Sets up pairings vector
    std::vector<std::vector<std::string>> pairings;
    for(int i = 0; i < numPlayers; i++) {
        pairings.push_back({});
    }    

    //Sets up pairings; has each player pair up to twice, as usually an extra player will fill up the third slot
    for(std::vector<std::string> playerVec : rankedPlayers) {
        std::string playerName = playerVec.at(1);
        int playerIndex = getPlayerIndex(rankedPlayers, playerName);
        int index = 0;
        int numPairings = pairings.at(playerIndex).size();
        for(std::vector<std::string> opponentVec : rankedPlayers) {
            std::string opponentName = opponentVec.at(1);
            int opponentIndex = getPlayerIndex(rankedPlayers, opponentName);
            if(numPairings < 2) {
                if(std::find(pairings.at(playerIndex).begin(), pairings.at(playerIndex).end(), opponentName) == pairings.at(playerIndex).end() && opponentName != playerName && std::find(oldPairings.at(playerIndex).begin(), oldPairings.at(playerIndex).end(), opponentName) == oldPairings.at(playerIndex).end() && pairings.at(opponentIndex).size() < 3) {
                    pairings.at(playerIndex).push_back(opponentName);
                    pairings.at(opponentIndex).push_back(playerName);
                    numPairings++;
                }
            }
        }
    }

    /*for(std::vector<std::string> pairing : pairings) {
        std::cout << "new pairings vector" << std::endl;
        for(std::string player : pairing) {
            std::cout << player << std::endl;
        }
    }*/

    //Convoluted code to fix byes
    //Checks the next lowest player above the first player with a bye; if possible, takes their lowest matchup and gives it to the second
    //player with a bye. Then the highest player with a bye plays that play that just lost a matchup. If this doesn't work, it increments
    //to check 2 ranks above, and repeats until it finds a valid pairing swap.
    int totalByes = 2;
    while(totalByes >= 2) {
        int highestRankWithBye = rankedPlayers.size() + 1;
        int secondHighestRankWithBye = rankedPlayers.size() + 1;
        totalByes = 0;
        for(int i = 0; i < pairings.size(); i++) {
            if(pairings.at(i).size() != 3) {
                totalByes += 3 - pairings.at(i).size();
                if(i < highestRankWithBye) {
                    secondHighestRankWithBye = highestRankWithBye;
                    highestRankWithBye = i;
                }
                else if(i < secondHighestRankWithBye) {
                    secondHighestRankWithBye = i;
                }
            }
        }
        //std::cout << highestRankWithBye << std::endl;
        //std::cout << secondHighestRankWithBye << std::endl;
        int numToSubtract = 1;
        bool matchesFound = false;
        if(totalByes >= 2) {
            if(secondHighestRankWithBye < rankedPlayers.size()) {
                while(!matchesFound) {
                    std::string highestPlayerWithBye = rankedPlayers.at(highestRankWithBye).at(1);
                    std::string secondHighestPlayerWithBye = rankedPlayers.at(secondHighestRankWithBye).at(1);
                    std::string nextHighestPlayer = rankedPlayers.at(highestRankWithBye - numToSubtract).at(1);
                    int highestByeIndex = getPlayerIndex(rankedPlayers, highestPlayerWithBye);
                    int nextHighestIndex = highestByeIndex - numToSubtract;
                    int secondHighestByeIndex = getPlayerIndex(rankedPlayers, secondHighestPlayerWithBye);
                    int lowestMatchupRank = 0;
                    for(std::string matchup : pairings.at(nextHighestIndex)) {
                        int rank = getPlayerIndex(rankedPlayers, matchup);
                        if(rank > lowestMatchupRank) {
                            lowestMatchupRank = rank;
                        }
                    }
                    std::string lowestMatchup = rankedPlayers.at(lowestMatchupRank).at(1);
                    if(std::find(oldPairings.at(highestByeIndex).begin(), oldPairings.at(highestByeIndex).end(), nextHighestPlayer) == oldPairings.at(highestByeIndex).end() && std::find(oldPairings.at(secondHighestByeIndex).begin(), oldPairings.at(secondHighestByeIndex).end(), lowestMatchup) == oldPairings.at(secondHighestByeIndex).end()) {
                        if(std::find(pairings.at(nextHighestIndex).begin(), pairings.at(nextHighestIndex).end(), highestPlayerWithBye) == pairings.at(nextHighestIndex).end() && std::find(pairings.at(secondHighestByeIndex).begin(), pairings.at(secondHighestByeIndex).end(), lowestMatchup) == pairings.at(secondHighestByeIndex).end()) {
                            int matchupIndex = std::find(pairings.at(nextHighestIndex).begin(), pairings.at(nextHighestIndex).end(), lowestMatchup) - pairings.at(nextHighestIndex).begin();
                            pairings.at(nextHighestIndex).erase(pairings.at(nextHighestIndex).begin() + matchupIndex);
                            int matchupIndex2 = std::find(pairings.at(lowestMatchupRank).begin(), pairings.at(lowestMatchupRank).end(), nextHighestPlayer) - pairings.at(lowestMatchupRank).begin();
                            pairings.at(lowestMatchupRank).erase(pairings.at(lowestMatchupRank).begin() + matchupIndex2);
                            pairings.at(highestByeIndex).push_back(nextHighestPlayer);
                            pairings.at(nextHighestIndex).push_back(highestPlayerWithBye);
                            pairings.at(secondHighestByeIndex).push_back(lowestMatchup);
                            pairings.at(lowestMatchupRank).push_back(secondHighestPlayerWithBye);
                            totalByes = totalByes - 2;
                            matchesFound = true;
                        }
                        else {
                            numToSubtract++;
                        }
                    }
                    else {
                        numToSubtract++;
                    }
                }
            }
            else {
                while(!matchesFound) {
                    std::string highestPlayerWithBye = rankedPlayers.at(highestRankWithBye).at(1);
                    std::string nextHighestPlayer = rankedPlayers.at(highestRankWithBye - numToSubtract).at(1);
                    int highestByeIndex = getPlayerIndex(rankedPlayers, highestPlayerWithBye);
                    int nextHighestIndex = highestByeIndex - numToSubtract;
                    int lowestMatchupRank = 0;
                    for(std::string matchup : pairings.at(nextHighestIndex)) {
                        int rank = getPlayerIndex(rankedPlayers, matchup);
                        if(rank > lowestMatchupRank) {
                            lowestMatchupRank = rank;
                        }
                    }
                    std::string lowestMatchup = rankedPlayers.at(lowestMatchupRank).at(1);
                    if(std::find(oldPairings.at(highestByeIndex).begin(), oldPairings.at(highestByeIndex).end(), nextHighestPlayer) == oldPairings.at(highestByeIndex).end() && std::find(oldPairings.at(highestByeIndex).begin(), oldPairings.at(highestByeIndex).end(), lowestMatchup) == oldPairings.at(highestByeIndex).end()) {
                        if(std::find(pairings.at(nextHighestIndex).begin(), pairings.at(nextHighestIndex).end(), highestPlayerWithBye) == pairings.at(nextHighestIndex).end() && std::find(pairings.at(lowestMatchupRank).begin(), pairings.at(lowestMatchupRank).end(), highestPlayerWithBye) == pairings.at(lowestMatchupRank).end()) {
                            int matchupIndex = std::find(pairings.at(nextHighestIndex).begin(), pairings.at(nextHighestIndex).end(), lowestMatchup) - pairings.at(nextHighestIndex).begin();
                            pairings.at(nextHighestIndex).erase(pairings.at(nextHighestIndex).begin() + matchupIndex);
                            int matchupIndex2 = std::find(pairings.at(lowestMatchupRank).begin(), pairings.at(lowestMatchupRank).end(), nextHighestPlayer) - pairings.at(lowestMatchupRank).begin();
                            pairings.at(lowestMatchupRank).erase(pairings.at(lowestMatchupRank).begin() + matchupIndex2);
                            pairings.at(highestByeIndex).push_back(nextHighestPlayer);
                            pairings.at(nextHighestIndex).push_back(highestPlayerWithBye);
                            pairings.at(lowestMatchupRank).push_back(highestPlayerWithBye);
                            pairings.at(highestByeIndex).push_back(lowestMatchup);
                            totalByes = totalByes - 2;
                            matchesFound = true;
                        }
                        else {
                            numToSubtract++;
                        }
                    }
                    else {
                        numToSubtract++;
                    }
                }
            }
        }
        /*std::cout << totalByes << std::endl;
        for(std::vector<std::string> pairing : pairings) {
            std::cout << "new pairings vector" << std::endl;
            for(std::string player : pairing) {
                std::cout << player << std::endl;
            }
        }*/
    }
    
    //Writes pairings to .csv file
    std::ofstream outputSheet;
    outputSheet.open("pairings.csv");

    for(std::vector<std::string> playerVec : rankedPlayers) {
        std::string playerName = playerVec.at(1);
        int playerIndex = getPlayerIndex(rankedPlayers, playerName);
        outputSheet << playerName + ",";
        for(std::string pairing : pairings.at(playerIndex)) {
            outputSheet << pairing + ",";
        }
        outputSheet << "\n";
    }
}
