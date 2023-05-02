//this file is not being used currently

#include <string>
class Sensor {
  public:
    Sensor(std::string measurement, std::string* tag, int n_tags,std::string* field,int n_fields) {
      this->measurement=measurement;
      this->point = new Point(string_to_char(measurement));
      //copy tags from parameter to local variable
      this->n_tags = n_tags;
      tags = new std::string[n_tags];
      for (int i = 0; i < n_tags; i++) tags[i] = tag[i];

      //copy fields from parameter to local variable
      this->n_fields = n_fields;
      fields = new std::string[n_fields];
      for (int i = 0; i < n_fields; i++) fields[i]=field[i];
    }

    char* string_to_char(std::string line){
        char* strc = new char[line.length() + 1];
        std::copy(line.begin(), line.end(), strc);
        strc[line.length()] = '\0';
        return strc;
    }
    char* string_to_char(char* line){
      return line;
    }

    void print_array(std::string* lines,int n_lines){
      for (int i = 0; i < n_lines; i++) Serial.println(string_to_char(lines[i]));
    }

    void test_tags(){
      print_array(this->tags,n_tags);
    }

    ~Sensor() {
      delete[] tags;
      delete[] fields;
    }
    // Resto de la definición de la clase...
  private:
    std::string measurement; 
    Point* point;
    std::string* tags; //array with all tags
    int n_tags;
    std::string* fields; //array with all fields
    int n_fields;
};