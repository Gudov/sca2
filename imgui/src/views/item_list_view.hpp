#include "view.hpp"

class ItemListView : public View {
  public:
    ItemListView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);
    void Update() override;

  private:
    static const size_t query_size = 1024;
    char query[query_size];
};