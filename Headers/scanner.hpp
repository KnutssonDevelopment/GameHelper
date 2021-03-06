#include <vector>
class Scanner{
    public:

        static std::vector<int> FirstDataScan(int numberType,int number);

    private:

        template<typename UNIT>
        static std::vector<int> InitialScan(int selectedProcess, UNIT n);
};