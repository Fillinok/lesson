/*
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }
    template <typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query, KeyMapper key_mapper) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, key_mapper);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        auto matched_documents = FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status2, int rating) { return status == status2; });        
        return matched_documents;
    }
    
    vector<Document> FindTopDocuments(const string& raw_query) const {
        auto matched_documents = FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });        
        return matched_documents;
    }
    
    

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    
    template <typename KeyMapper>
    vector<Document> FindAllDocuments(const Query& query, KeyMapper key_mapper) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {                
                if(key_mapper(document_id, documents_.at(document_id).status, documents_.at(document_id).rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }                                                    
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }               

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};

#define RUN_TEST(expr) RunTest(expr,#expr)
#define ASSERT(expr) Assert((expr), __FILE__ , __FUNCTION__ , __LINE__ )
#define ASSERT_EQUAL(expr1,expr2) AssertEqual (expr1,expr2, __FILE__ , __FUNCTION__ , __LINE__)
#define ASSERT_HINT(expr,hint) AssertHint (expr,__FILE__ , __FUNCTION__ , __LINE__,hint)
#define ASSERT_EQUAL_HINT(expr1,expr2,hint) AssertEqualHint (expr1,expr2,__FILE__ , __FUNCTION__ , __LINE__,hint)


template <typename TestFunc>
void RunTest(const TestFunc& func, const string& test_name) {
    func();
    cerr << test_name << " OK"s << endl;
}

template <typename T>
void Assert(const T& t, const string& file, const string& function, unsigned line) {
    if(!t)
    {
        cout << "Test falier: "s << file << "("s << line << ")"s << "  "s << function << "()"s <<  endl;
    }       
}

template <typename T, typename U>
void AssertEqual(const T& t, const U& u, const string& file, const string& function, unsigned line) {
    if (t != u) {
        cout << "Assertion failed: "s << t << " != "s << u << " in "s << function << endl;        
    }    
}

template <typename T>
void AssertHint (const T& t, const string& file, const string& function, unsigned line , const string& hint) {
    if(!t)
    {
        cout << endl;
        cout << "Test falier: "s << file << "("s << line << ")"s << "  "s << function << "()"s <<  endl;
        cout << "Hint : "s << hint << endl;
        cout << endl;
    }    
}
template <typename T, typename U>
void AssertEqualHint (const T& t, const U& u, const string& file, const string& function, unsigned line , const string& hint) {
    if(t != u)
    {
        cout << endl;
        cout << "Failed: "s << t << " != "s << u << " in ........ "s << file << "("s << line << ")"s << "  "s << function << "()"s <<  endl;
        cout << "Hint : "s << hint << endl;
        cout << endl;
    }    
}
//*/


// -------- Начало модульных тестов поисковой системы ----------





// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u,"not pass");
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto result = server.FindTopDocuments("city");
        ASSERT_EQUAL(result.size(),1);
        ASSERT_EQUAL(result[0].id,42);   
    }
}

void MinusWordsAreNotParsed(){
    const int doc_id_1 = 1, doc_id_2 = 2, doc_id_3 = 3;
    const string content_1 = "cat the city"s, content_2 = " white cat in box"s, content_3 = "dog sleep in box"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const vector<int> ratings_2 = {2, 3, 8};
    const vector<int> ratings_3 = {-6, 3, 0};
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        const auto result = server.FindTopDocuments("box -cat");    
        ASSERT_EQUAL_HINT(result.size(),1,"Search have minusword"s);
        ASSERT(!result.empty());
    }
}

int AverageRating(const vector<int>& v) {
    int result = 0;    
    for (int i : v) 
    {
       result += i;  
    }
    int av_rat = result/static_cast<int>(v.size());
    return av_rat;
}

void TestRating(){    
    const int doc_id_1 = 10, doc_id_2 = 20, doc_id_3 = 30;
    const string content_1 = "rat in the box"s, content_2 = "white cat in box"s, content_3 = "dog sleep in box"s;
    const vector<int> ratings_1 = {1, 2, 3};
    const vector<int> ratings_2 = {2, 3, 7};
    const vector<int> ratings_3 = {-6, 3, 0};    
    int av_rat_1 = AverageRating(ratings_1);
    int av_rat_2 = AverageRating(ratings_2);
    int av_rat_3 = AverageRating(ratings_3);    
    string query = "box"s;
    {
        SearchServer server;               
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        vector<Document> result = server.FindTopDocuments(query);        
        ASSERT_EQUAL_HINT(result[0].rating,av_rat_2,"Rating 1 wrong"s);           
        ASSERT_EQUAL_HINT(result[1].rating,av_rat_1,"Rating 2 wrong"s);
        ASSERT_EQUAL_HINT(result[2].rating,av_rat_3,"Rating 3 wrong"s);        
    }
    
}

/*
Разместите код остальных тестов здесь
*/


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);    
    RUN_TEST(MinusWordsAreNotParsed);
    //RUN_TEST(TestRating);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
