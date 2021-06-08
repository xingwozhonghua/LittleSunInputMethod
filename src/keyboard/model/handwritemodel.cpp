#include "handwritemodel.h"
#include <fstream>
#include <cmath>
#include <regex>
#include <string>
#include <QFile>
#include <QDebug>

HandWriteModel::HandWriteModel()
{

}

bool HandWriteModel::loadModelFileNew(const char* filePath, int charType)
{
    QFile ifs(filePath);

    if(!ifs.open(QIODevice::ReadOnly)){
        perror("open");
        return false;
    }
    QTextStream in(&ifs);
    in.setCodec("GBK");     //以GBK的编码读取字符串

    QList<CharacterItem>* cItems;
    if (charType == CHAR_CHINESE){
        cItems = &charItems;
    } else if (charType == CHAR_NUM){
        cItems = &numItems;
    } else {
        return  false;
    }


    while(!ifs.atEnd()){

        QString line = ifs.readLine();
        if (line.length() == 0){
            continue;
        }
//        qDebug() << line;
        CharacterItem charItem;
        QStringList chs = line.split(":");
        charItem.word = chs[0];
        for (int i = 1; i < chs.size(); ++i){
            CharacterEntity character;
            character.strokeCount = 0;
            QStringList strokeList = chs[i].split("|");
//            character.text = strokeList[0];
    //        qDebug() << character.text;
            for (int i = 0; i < strokeList.length(); ++i){
                QStringList points = strokeList.at(i).split(",");
                StrokeEntity strocke;
                for(QString d : points){
                    PointEntity point;
                    point.direc = d.toDouble();
                    strocke.points.push_back(point);
                }
                character.strokes.push_back(strocke);
                character.strokeCount++;
            }
            charItem.charItem.push_back(character);
        }
//        qDebug() << charItem.toDireString();
        cItems->push_back(charItem);
    }
    return true;
}

static bool cmpWordDist(const WordEntity word1, const WordEntity word2)
{
    return word1.dist < word2.dist;
}

bool HandWriteModel::recognize(CharacterEntity character, QStringList* resultWords)
{
    if(character.strokeCount == 0)
        return false;
    getTurnPoints(&character);
    norm(&character);
    QList<WordEntity> words;

    QList<CharacterItem>* cItems;
    if (!character.isNum){
        cItems = &charItems;
    } else {
        cItems = &numItems;
    }

    for(unsigned int i = 0; i < cItems->size(); ++i){
        int mdist = 100000;
        for (int j = 0; j < cItems->at(i).charItem.size(); ++j){
            CharacterEntity tmpCharacter = cItems->at(i).charItem[j];
            int d = dist(&character, &tmpCharacter);
            if (d >= 0 && d < mdist){
                mdist = d;
            }
            qDebug() << "d: " << d;
        }

        WordEntity word;
        word.word = cItems->at(i).word;
        word.dist = mdist;
        if(word.dist < 100000){
            words.push_back(word);
        }
    }

    std::sort(words.begin(), words.end(), cmpWordDist);
    for(unsigned int i = 0; i < words.size(); ++i){
        WordEntity word = words[i];
        if (i < 5 )
            qDebug() << word.word << word.dist;
        resultWords->push_back(word.word);
    }
    return true;
}

double HandWriteModel::dist(const CharacterEntity* character1, const CharacterEntity* character2)
{
    double dist = MAXDIST;
    if(character2->strokeCount >= character1->strokeCount && character2->strokeCount <= character1->strokeCount + 2){
        double allStrokeDist = 0.0f;
        for(int i = 0; i < character1->strokeCount; ++i){
            StrokeEntity stroke1 = character1->strokes[i];
            StrokeEntity stroke2 = character2->strokes[i];
            double strokeDist = distBetweenStrokes(stroke1, stroke2);

            allStrokeDist += strokeDist;

            if(strokeDist > MAX_DIFF_PER_STROKE){
                allStrokeDist = MAXDIST;
                return allStrokeDist;
            }
        }
        // 笔画更接近的优先级更高
        return allStrokeDist / character1->strokeCount + (character2->strokeCount - character1->strokeCount)*10000/character1->strokeCount;
    }
    return dist;
}

double HandWriteModel::distBetweenStrokes(const StrokeEntity stroke1, const StrokeEntity stroke2)
{
    double strokeDist = MAXDIST;
    double dist = 0.0f;
    int minLength = fmin(stroke1.points.size(), stroke2.points.size());
    StrokeEntity largeStroke = stroke1.points.size() > minLength ? stroke1 : stroke2;
    StrokeEntity smallStroke = stroke1.points.size() > minLength ? stroke2 : stroke1;


    for(int j = 0; j < minLength-1; ++j){
        double diretion1 = largeStroke.points[j].direc;
        double diretion2 = smallStroke.points[j].direc;
        double d = fabs(diretion1 - diretion2);
        if (d > 18000){
            d = 36000 - d;
        }
        dist += d;


    }
    for (int j = minLength-1; j < largeStroke.points.size()-1; ++j){
        double d = fabs(largeStroke.points[j].direc - smallStroke.points[minLength-1].direc);
        if (d > 18000){
            d = 36000 - d;
        }
        dist += d;
    }

    double d = fabs(largeStroke.points[largeStroke.points.size()-1].direc - smallStroke.points[smallStroke.points.size()-1].direc);
    if (d > 18000){
        d = 36000 - d;
    }
    dist += d;

//    qDebug() << "偏差值: " << dist;

    strokeDist = dist / minLength;
    return strokeDist;
}

void HandWriteModel::getTurnPoints(CharacterEntity* character)
{
    for(int i = 0; i < character->strokeCount; ++i){
         StrokeEntity *stroke = &character->strokes[i];
         if(stroke->points.size() > 1){
             std::vector<PointEntity> points;
             points.push_back(stroke->points[0]);
             turnPoints(stroke, &points, 0, (int)stroke->points.size() - 1);
             points.push_back(stroke->points[stroke->points.size() - 1]);
             stroke->points.clear();
             for(unsigned int i = 0; i < points.size(); ++i){
                 stroke->points.push_back(points[i]);
             }
         }
     }
}

void HandWriteModel::turnPoints(StrokeEntity *stroke, std::vector<PointEntity> *points, int pointIndex1, int pointIndex2)
{
    if(pointIndex1 < 0 || pointIndex2 <= 0 || pointIndex1 >= pointIndex2 - 1)
        return;
    const float b = stroke->points[pointIndex1].x - stroke->points[pointIndex2].x;
    const float a = stroke->points[pointIndex2].y - stroke->points[pointIndex1].y;
    const float c = stroke->points[pointIndex1].x * a + stroke->points[pointIndex1].y * b;
    float len = sqrt(a*a + b*b)/2;
    float max = 0.17632698;    //tan(10°)
    int maxDistPointIndex = -1;
    for(int i = pointIndex1 + 1; i < pointIndex2 && len > 2; ++i){
        PointEntity point = stroke->points[i];
        float h = abs(a * point.x + b * point.y - c)/sqrt(a*a + b*b);
        const float dist = h/len;
        if (dist > max) {
            max = dist;
            maxDistPointIndex = i;
        }
    }
    if(maxDistPointIndex != -1){
        turnPoints(stroke, points, pointIndex1, maxDistPointIndex);
        points->push_back(stroke->points[maxDistPointIndex]);
        turnPoints(stroke, points, maxDistPointIndex, pointIndex2);
    }
}

void HandWriteModel::norm(CharacterEntity* character)
{
    PointEntity lastPoint(-1, -1);
    PointEntity lastStrick(-1, -1);
    for(int i = 0; i < character->strokes.size(); ++i){
        StrokeEntity stroke = character->strokes[i];
        PointEntity tmpPoint(-1, -1);
        for(unsigned int j = 0; j < stroke.points.size(); ++j){
            tmpPoint = stroke.points[j];
            if (lastPoint.x == -1 && lastPoint.y == -1){
                character->strokes[i].points[j].direc = 0;

            } else {
                character->strokes[i].points[j].setDire(lastPoint);
            }
            lastPoint = tmpPoint;
        }
        PointEntity point;
        if (i == 0){
            point.direc = 0;
        } else {
            point.direc = PointEntity::setDire(character->strokes[i-1].points[0], character->strokes[i].points[0]);
        }
        lastStrick = lastPoint;
        character->strokes[i].points.push_back(point);
    }
}