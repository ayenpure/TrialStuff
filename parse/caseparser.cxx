#include <boost/algorithm/string.hpp>

#include<cstdlib>
#include<cstring>
#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include<vector>


void trimString(std::string& str)
{
  boost::trim(str);
}

int MapVisitToVTKm(std::vector<std::string>& tokens,
                    std::vector<int>& splitwiki)
{
  const std::string shape("ST_");
  const std::string edge("E");
  const std::string vertex("P");
  const std::string point("N");

  /*ST_PNT, 0, NOCOLOR, 5, EI, EJ, ED, EC, EL,
  ST_PYR, COLOR1, EJ, EL, P2, P1, N0*/
  /* P0-P7 -> Vertices for the shape already present */
  /* E{A-L} -> Edge for interpolation of new points */
  /* N0 -> new point to be interpolated inside the cell */
  /*
    Available shapes, written in increasing sequence of
    number of vertices

    ST_VTX -> vtkm::CELL_SHAPE_VERTEX (1)
    ST_PNT -> vtkm::CELL_SHAPE_VERTEX (1)
    ST_LIN -> vtkm::CELL_SHAPE_LINE (3)
    ST_TRI -> vtkm::CELL_SHAPE_TRIANGLE (5)
    ST_QUA -> vtkm::CELL_SHAPE_QUAD (9)
    ST_TET -> vtkm::CELL_SHAPE_TETRA (10)
    ST_PYR -> vtkm::CELL_SHAPE_PYRAMID (14)
    ST_WDG -> vtkm::CELL_SHAPE_WEDGE (13)
    ST_HEX -> vtkm::CELL_SHAPE_HEXAHEDRON (12)
  */
  int i = 0;
  int size = tokens.size();
  for(;i < tokens.size();)
  {
    std::string token = tokens[i];
    trimString(token);
    if(token.find(shape) != std::string::npos)
    {
      if(token.find("ST_VTX") != std::string::npos)
      { // should not be required
      }
      else if(token.find("ST_PNT") != std::string::npos)
      { size -= 4; i+=4; splitwiki.push_back(1); splitwiki.push_back(size);}
      else if(token.find("ST_LIN") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(3); splitwiki.push_back(size);}
      else if(token.find("ST_TRI") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(4); splitwiki.push_back(size);}
      else if(token.find("ST_QUA") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(9); splitwiki.push_back(size);}
      else if(token.find("ST_TET") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(10); splitwiki.push_back(size);}
      else if(token.find("ST_PYR") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(14); splitwiki.push_back(size);}
      else if(token.find("ST_WDG") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(13); splitwiki.push_back(size);}
      else if(token.find("ST_HEX") != std::string::npos)
      { size -= 2; i+=2; splitwiki.push_back(12); splitwiki.push_back(size);}
    }
    else if(token.find(edge) != std::string::npos)
    {
      //Write corresponding edge to vector
      if((token.c_str())[1] == 'A')
      { splitwiki.push_back(0);}
      else if((token.c_str())[1] == 'B')
      { splitwiki.push_back(1);}
      else if((token.c_str())[1] == 'C')
      { splitwiki.push_back(2);}
      else if((token.c_str())[1] == 'D')
      { splitwiki.push_back(3);}
      else if((token.c_str())[1] == 'E')
      { splitwiki.push_back(4);}
      else if((token.c_str())[1] == 'F')
      { splitwiki.push_back(5);}
      else if((token.c_str())[1] == 'G')
      { splitwiki.push_back(6);}
      else if((token.c_str())[1] == 'H')
      { splitwiki.push_back(7);}
      else if((token.c_str())[1] == 'I')
      { splitwiki.push_back(8);}
      else if((token.c_str())[1] == 'J')
      { splitwiki.push_back(9);}
      else if((token.c_str())[1] == 'K')
      { splitwiki.push_back(10);}
      else if((token.c_str())[1] == 'L')
      { splitwiki.push_back(11);}
      ++i;
    }
    else if(token.find(vertex) != std::string::npos)
    {
      //Write corresponding vertex to vector
      if((token.c_str())[1] == '0')
      { splitwiki.push_back(100);}
      else if((token.c_str())[1] == '1')
      { splitwiki.push_back(101);}
      else if((token.c_str())[1] == '2')
      { splitwiki.push_back(102);}
      else if((token.c_str())[1] == '3')
      { splitwiki.push_back(103);}
      else if((token.c_str())[1] == '4')
      { splitwiki.push_back(104);}
      else if((token.c_str())[1] == '5')
      { splitwiki.push_back(105);}
      else if((token.c_str())[1] == '6')
      { splitwiki.push_back(106);}
      else if((token.c_str())[1] == '7')
      { splitwiki.push_back(107);}
      ++i;
    }
    else if(token.find(point) != std::string::npos)
    {
      // Write new point to vector
      // We need to interpolate using all the points in front of ST_PNT above
      // They may be edges, or vertices
      splitwiki.push_back(200);
      ++i;
    }
  }
  return size + 2;
}

void ParseForData(const std::string& filename,
                  std::vector<int>& numShapes,
                  std::vector<int>& splitWiki,
                  std::vector<int>& caseOffsets)
{
  // Start counting when you encounter this.
  const std::string startCue("clipShapesHex");
  // Stop counting when you encounter this.
  const std::string stopCue("// Dummy");
  const std::string newCase("//");
  const std::string delim(",");
  const char *delimchar = delim.c_str();
  std::string currentRead;
  std::ifstream stream;
  stream.open(filename.c_str());
  if(!stream)
  {
    std::cerr << "Error reading file" << std::endl;
    return;
  }
  while(std::getline(stream, currentRead) &&
        currentRead.find(startCue) == std::string::npos)
   continue;
  int caseid = 0;
  int count  = 0;
  int offset = 0;
  std::vector<int> currentsplits;
  while(std::getline(stream, currentRead))
  {
    trimString(currentRead);
    //std::cout << "Currently processing : " << currentRead << std::endl;
    if(currentRead.find(newCase) != std::string::npos)
    {
      if(caseid != 0)
      {
        caseOffsets.push_back(offset);
        // For each case we include num of shapes in the wiki
        ++offset;
        numShapes.push_back(count);
        splitWiki.push_back(count);
        if(currentsplits.size() != 0)
          splitWiki.insert(splitWiki.begin() + splitWiki.size(),
                          currentsplits.begin(), currentsplits.end());
        currentsplits.clear();
      }
      // Condition to quit taking cases.
      if(currentRead.find(stopCue) != std::string::npos)
        break;
      count = 0;
      ++caseid;
    }
    else
    {
      // Tokenize the string on commas
      std::vector<std::string> tokens;
      char* toTokenize = strdup(currentRead.c_str());
      char * token = std::strtok(toTokenize, delimchar);
      while(token != NULL)
      {
        tokens.push_back(std::string(token));
        token = std::strtok(NULL, delimchar);
      }
      offset += MapVisitToVTKm(tokens, currentsplits);
      ++count;
    }
  }
}

int main(int argc, char **argv)
{
  if(argc < 2)
  {
     std::cout << "Incorrect number of parameters" << std::endl;
  }
  const std::string filename(argv[1]);
  std::vector<int> numShapes;
  std::vector<int> splitWiki;
  std::vector<int> caseOffsets;
  ParseForData(filename, numShapes, splitWiki, caseOffsets);

  // print offsets.
  /* for(int i = 0; i < 256; i++)
  {
    std::cout << "Case : " << i << ", Num Shapes : " << numShapes[i] << ", Offset : "
      << caseOffsets[i] << std::endl;
  }*/
  std::cout << numShapes.size() << ", " << caseOffsets.size() << std::endl;
  for(int i = 0; i < 256; i++)
  {
    int start = caseOffsets[i];
    int end = caseOffsets[i+1];
    for(int j = start; j < end; j++)
    {
      std::cout << "Case " << i << ", Shapes : " << splitWiki[j] << std::endl;
      break;
    }
  }
}
