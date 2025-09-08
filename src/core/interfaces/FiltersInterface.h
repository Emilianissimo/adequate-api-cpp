class FiltersInterface {
protected:
    virtual void parseRequestQuery(std::unordered_map<std::string, std::string> query) {}
};
