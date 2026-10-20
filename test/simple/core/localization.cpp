// doctest
#include <doctest/doctest.h>

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/localization.hpp>

// test
#include "support/temp_tree.hpp"


using test::TempTree;


namespace {
    // ru has everything, en lacks mail/sub/footer.html
    void fill(const TempTree& tree) {
        tree.write("ru/mail/new_order.txt", "Новый заказ");
        tree.write("en/mail/new_order.txt", "New order");
        tree.write("ru/mail/sub/footer.html", "<p>Подвал</p>");

        tree.write("ru/pages/about.html", "О нас");
        tree.write("en/pages/about.html", "About");
    }

    ers::FolderPtr folder_of(const ers::LocaleTable& table, std::string_view name) {
        const auto index = table.index();
        const auto it = index->folders.find(name);

        return it != index->folders.end() ? it->second : nullptr;
    }
}


// Keys ------------------------------------------------------------------------

TEST_CASE("localization: keys are made of extension, subfolders and file name") {
    CHECK(*ers::make_localization_key("new_order.txt") == "txt.new_order");
    CHECK(*ers::make_localization_key("sub/deep/footer.html") == "html.sub.deep.footer");
    CHECK(*ers::make_localization_key("a.b.txt") == "txt.a.b");

    CHECK_FALSE(ers::make_localization_key("no_extension"));
}


// Loading ---------------------------------------------------------------------

TEST_CASE("localization: loads every locale and folder") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail", "pages" });
    CHECK(report->kept == 0);
    CHECK(report->removed.empty());

    const auto config = table.config();

    CHECK(config->locales == std::vector<std::string> { "en", "ru" });
    CHECK(config->fallback_locale_id == *config->id_of("ru"));

    CHECK(table["ru"]["mail"]["txt.new_order"] == "Новый заказ");
    CHECK(table["en"]["mail"]["txt.new_order"] == "New order");
    CHECK(table["ru"]["mail"]["html.sub.footer"] == "<p>Подвал</p>");
    CHECK(table["en"]["pages"]["html.about"] == "About");
}

TEST_CASE("localization: a missing text falls back, but is still reported") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    CHECK(table["en"]["mail"]["html.sub.footer"] == "<p>Подвал</p>");
    CHECK(table.missing() == std::vector<std::string> { "en: mail/html.sub.footer" });
}

TEST_CASE("localization: unknown names") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    CHECK_FALSE(table.find("de"));
    CHECK_THROWS(static_cast<void>(table["de"]));

    CHECK_FALSE(table["ru"].find("nope"));
    CHECK_THROWS(static_cast<void>(table["ru"]["nope"]));

    CHECK_FALSE(table["ru"]["mail"].find("txt.nope"));
    CHECK_THROWS(static_cast<void>(table["ru"]["mail"]["txt.nope"]));
    CHECK(table["ru"]["mail"].share("txt.nope") == nullptr);
}

TEST_CASE("localization: fails without the fallback locale or a root") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;

    CHECK_FALSE(table.reload(tree.root(), "de"));
    CHECK_FALSE(table.reload(tree.root() / "nope", "ru"));
    CHECK_FALSE(table.find("ru"));
}


// Reloading -------------------------------------------------------------------

TEST_CASE("localization: reload keeps untouched folders as they are") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    const auto mail = folder_of(table, "mail");
    const auto pages = folder_of(table, "pages");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded.empty());
    CHECK(report->kept == 2);

    CHECK(folder_of(table, "mail") == mail);
    CHECK(folder_of(table, "pages") == pages);
}

TEST_CASE("localization: reload keeps folders touched without changes") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    const auto pages = folder_of(table, "pages");

    tree.write("en/pages/about.html", "About");
    tree.touch("en/pages/about.html");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded.empty());
    CHECK(report->kept == 2);
    CHECK(folder_of(table, "pages") == pages);
}

TEST_CASE("localization: reload rebuilds only the changed folder") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    const auto mail = folder_of(table, "mail");
    const auto pages = folder_of(table, "pages");

    const auto old_view = table["en"]["pages"];
    const auto old_text = old_view.share("html.about");

    // Same size on purpose, only the time and the checksum tell it apart.
    tree.write("en/pages/about.html", "Abort");
    tree.touch("en/pages/about.html");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "pages" });
    CHECK(report->kept == 1);

    CHECK(folder_of(table, "mail") == mail);
    CHECK(folder_of(table, "pages") != pages);

    CHECK(table["en"]["pages"]["html.about"] == "Abort");

    // Whoever held the old folder still sees it.
    CHECK(old_view["html.about"] == "About");
    CHECK(*old_text == "About");
}

TEST_CASE("localization: reload drops removed folders") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    tree.remove("ru/pages");
    tree.remove("en/pages");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->removed == std::vector<std::string> { "pages" });
    CHECK(report->kept == 1);
    CHECK_FALSE(table["ru"].find("pages"));
}

TEST_CASE("localization: a new locale rebuilds every folder") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    tree.write("de/mail/new_order.txt", "Neue Bestellung");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail", "pages" });
    CHECK(report->kept == 0);

    CHECK(table["de"]["mail"]["txt.new_order"] == "Neue Bestellung");
    CHECK(table["de"]["pages"]["html.about"] == "О нас");
}

TEST_CASE("localization: a failed reload keeps the previous table") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table;
    REQUIRE(table.reload(tree.root(), "ru"));

    // Both become "txt.a.b".
    tree.write("ru/mail/a.b.txt", "first");
    tree.write("ru/mail/a/b.txt", "second");

    CHECK_FALSE(table.reload(tree.root(), "ru"));
    CHECK(table["ru"]["mail"]["txt.new_order"] == "Новый заказ");
    CHECK_FALSE(table["ru"]["mail"].find("txt.a.b"));

    // The failed attempt must not leave stale fingerprints behind.
    tree.remove("ru/mail/a");

    const auto report = table.reload(tree.root(), "ru");

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail" });
    CHECK(table["ru"]["mail"]["txt.a.b"] == "first");
}
