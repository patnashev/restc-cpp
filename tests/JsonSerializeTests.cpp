
// Include before boost::log headers
#include "restc-cpp/logging.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/fusion/adapted.hpp>

#include "restc-cpp/restc-cpp.h"
#include "restc-cpp/SerializeJson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "UnitTest++/UnitTest++.h"


using namespace std;
using namespace restc_cpp;
using namespace rapidjson;

struct Person {

    Person(int id_, std::string name_, double balance_)
    : id{id_}, name{std::move(name_)}, balance{balance_}
    {}

    Person() = default;
    Person(const Person&) = default;
    Person(Person&&) = default;

    int id = 0;
    std::string name;
    double balance = 0;
};

BOOST_FUSION_ADAPT_STRUCT(
    Person,
    (int, id)
    (std::string, name)
    (double, balance)
)

struct Quotes {
    int id;
    string origin;
    string quote;
};

BOOST_FUSION_ADAPT_STRUCT(
    Quotes,
    (int, id)
    (std::string, origin)
    (std::string, quote)
)



struct Group {

    Group(std::string name_, int gid_, Person leader_,
	  std::vector<Person> members_ = {},
          std::list<Person> more_members_ = {},
          std::deque<Person> even_more_members_ = {})
    : name{std::move(name_)}, gid{gid_}, leader{std::move(leader_)}
    , members{move(members_)}, more_members{move(more_members_)}
    , even_more_members{move(even_more_members_)}
    {}

    Group() = default;
    Group(const Group&) = default;
    Group(Group&&) = default;

    std::string name;
    int gid = 0;

    Person leader;
    std::vector<Person> members;
    std::list<Person> more_members;
    std::deque<Person> even_more_members;
};

BOOST_FUSION_ADAPT_STRUCT(
    Group,
    (std::string, name)
    (int, gid)
    (Person, leader)
    (std::vector<Person>, members)
    (std::list<Person>, more_members)
    (std::deque<Person>, even_more_members)
)

TEST(SerializeSimpleObject)
{
    Person person = { 100, "John Doe", 123.45 };

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    RapidJsonSerializer<decltype(person), decltype(writer)>
        serializer(person, writer);

    serializer.Serialize();

    CHECK_EQUAL(R"({"id":100,"name":"John Doe","balance":123.45})",
                s.GetString());

}

TEST(SerializeNestedObject)
{
    Group group = Group(string("Group name"), 99, Person( 100, string("John Doe"), 123.45 ));

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    RapidJsonSerializer<decltype(group), decltype(writer)>
        serializer(group, writer);

    serializer.IgnoreEmptyMembers(false);
    serializer.Serialize();

    CHECK_EQUAL(R"({"name":"Group name","gid":99,"leader":{"id":100,"name":"John Doe","balance":123.45},"members":[],"more_members":[],"even_more_members":[]})",
                s.GetString());

}

TEST(SerializeVector)
{
    std::vector<int> ints = {-1,2,3,4,5,6,7,8,9,-10};

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    RapidJsonSerializer<decltype(ints), decltype(writer)>
        serializer(ints, writer);

    serializer.Serialize();

    CHECK_EQUAL(R"([-1,2,3,4,5,6,7,8,9,-10])",
                s.GetString());

}

TEST(SerializeList)
{
    std::list<unsigned int> ints = {1,2,3,4,5,6,7,8,9,10};

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    RapidJsonSerializer<decltype(ints), decltype(writer)>
        serializer(ints, writer);

    serializer.Serialize();

    CHECK_EQUAL(R"([1,2,3,4,5,6,7,8,9,10])",
                s.GetString());

}

TEST(DeserializeSimpleObject)
{
    Person person;
    std::string json = R"({ "id" : 100, "name" : "John Longdue Doe", "balance" : 123.45 })";

    RapidJsonDeserializer<Person> handler(person);
    Reader reader;
    StringStream ss(json.c_str());
    reader.Parse(ss, handler);

    CHECK_EQUAL(person.id, 100);
    CHECK_EQUAL(person.name, "John Longdue Doe");
    CHECK_EQUAL(person.balance, 123.45);
}

TEST(DeserializeNestedObject)
{
    assert(boost::fusion::traits::is_sequence<Group>::value);
    assert(boost::fusion::traits::is_sequence<Person>::value);

    Group group;
    std::string json =
        R"({"name" : "qzar", "gid" : 1, "leader" : { "id" : 100, "name" : "Dolly Doe", "balance" : 123.45 },)"
        R"("members" : [{ "id" : 101, "name" : "m1", "balance" : 0.0}, { "id" : 102, "name" : "m2", "balance" : 1.0}],)"
        R"("more_members" : [{ "id" : 103, "name" : "m3", "balance" : 0.1}, { "id" : 104, "name" : "m4", "balance" : 2.0}],)"
        R"("even_more_members" : [{ "id" : 321, "name" : "m10", "balance" : 0.1}, { "id" : 322, "name" : "m11", "balance" : 22.0}])"
        R"(})";

    RapidJsonDeserializer<Group> handler(group);
    Reader reader;
    StringStream ss(json.c_str());
    reader.Parse(ss, handler);

    CHECK_EQUAL(1, group.gid);
    CHECK_EQUAL("qzar", group.name);
    CHECK_EQUAL(100, group.leader.id);
    CHECK_EQUAL("Dolly Doe", group.leader.name);
    CHECK_EQUAL(123.45, group.leader.balance);
    CHECK_EQUAL(2, static_cast<int>(group.members.size()));
    CHECK_EQUAL(101, group.members[0].id);
    CHECK_EQUAL("m1", group.members[0].name);
    CHECK_EQUAL(0.0, group.members[0].balance);
    CHECK_EQUAL(102, group.members[1].id);
    CHECK_EQUAL("m2", group.members[1].name);
    CHECK_EQUAL(1.0, group.members[1].balance);
    CHECK_EQUAL(2, static_cast<int>(group.more_members.size()));
    CHECK_EQUAL(103, group.more_members.front().id);
    CHECK_EQUAL("m3", group.more_members.front().name);
    CHECK_EQUAL(0.1, group.more_members.front().balance);
    CHECK_EQUAL(104, group.more_members.back().id);
    CHECK_EQUAL("m4", group.more_members.back().name);
    CHECK_EQUAL(2.0, group.more_members.back().balance);
    CHECK_EQUAL(321, group.even_more_members.front().id);
    CHECK_EQUAL("m10", group.even_more_members.front().name);
    CHECK_EQUAL(0.1, group.even_more_members.front().balance);
    CHECK_EQUAL(322, group.even_more_members.back().id);
    CHECK_EQUAL("m11", group.even_more_members.back().name);
    CHECK_EQUAL(22.0, group.even_more_members.back().balance);
}

TEST(DeserializeIntVector)
{
    std::string json = R"([1,2,3,4,5,6,7,8,9,10])";

    std::vector<int> ints;
    RapidJsonDeserializer<decltype(ints)> handler(ints);
    Reader reader;
    StringStream ss(json.c_str());
    reader.Parse(ss, handler);

    CHECK_EQUAL(10, static_cast<int>(ints.size()));

    auto val = 0;
    for(auto v : ints) {
        CHECK_EQUAL(++val, v);
    }
}

TEST(DeserializeMemoryLimit)
{

    Quotes q;
    q.origin = "HGG";
    q.quote = "For instance, on the planet Earth, man had always assumed that he was" "more intelligent than dolphins because he had achieved so much—the wheel, New " "York, wars and so on—whilst all the dolphins had ever done was muck about in the " "water having a good time. But conversely, the dolphins had always believed that " "they were far more intelligent than man—for precisely the same reasons.";

    std::list<Quotes> quotes;
    for(int i = 0; i < 100; ++i) {
        q.id = i;
        quotes.push_back(q);
    }

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    RapidJsonSerializer<decltype(quotes), decltype(writer)> serializer(quotes, writer);
    serializer.Serialize();

    std::string json = s.GetString();

    quotes.clear();

    // Add a limit of approx 4000 bytes to the handler
    RapidJsonDeserializer<decltype(quotes)> handler(quotes, 4000);
    Reader reader;
    StringStream ss(json.c_str());

    CHECK_THROW(reader.Parse(ss, handler), ConstraintException);
}

int main(int, const char *[])
{
    namespace logging = boost::log;
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::trace
    );
    return UnitTest::RunAllTests();
}

