// doctest
#include <doctest/doctest.h>

// std
#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/localization.hpp>

// test
#include "support/temp_tree.hpp"


using test::TempTree;

using std::string_literals::operator ""s;
using std::string_view_literals::operator ""sv;


namespace {
    // "key = text" per line: enough to test bundles without erslib::contrib
    ers::bundle_loader_t lines_loader() {
        return {
            .suffix = ".l10n.txt",
            .parse  = [](std::string_view text) -> ers::Result<std::vector<ers::bundle_entry_t>> {
                std::vector<ers::bundle_entry_t> out;

                while (!text.empty()) {
                    auto end = std::min(text.find('\n'), text.size());
                    auto line = text.substr(0, end);
                    text.remove_prefix(std::min(end + 1, text.size()));

                    if (line.empty())
                        continue;

                    auto at = line.find(" = ");

                    if (at == std::string_view::npos)
                        return ers::make_error("'{}' has no ' = '.", line);

                    out.push_back({ .key = std::string(line.substr(0, at)), .text = std::string(line.substr(at + 3)) });
                }

                return out;
            },
        };
    }

    ers::locale_table_options_t options(ers::EInvalidDomain on_invalid = ers::EInvalidDomain::KeepPrevious) {
        return {
            .fallback_locale = "ru",
            .bundles         = { lines_loader() },
            .on_invalid      = on_invalid,
        };
    }


    // ru has everything, en lacks mail/sub/footer.html
    void fill(const TempTree& tree) {
        tree.write("ru/mail/new_order.txt", "Новый заказ");
        tree.write("en/mail/new_order.txt", "New order");
        tree.write("ru/mail/sub/footer.html", "<p>Подвал</p>");

        tree.write("ru/pages/about.html", "О нас");
        tree.write("en/pages/about.html", "About");
    }

    ers::DomainPtr domain_of(const ers::LocaleTable& table, std::string_view name) {
        const auto index = table.index();
        const auto it = index->domains.find(name);

        return it != index->domains.end() ? it->second : nullptr;
    }


    // every colour needs a hex
    struct Palette {
        static constexpr std::string_view domain = "colors";

        static ers::Status validate(const ers::DomainPtr& colors) {
            const Palette palette(ers::DomainView(colors, colors->config().fallback_locale_id));

            for (auto key : palette.ids()) {
                if (!palette.hex(key))
                    return ers::make_error("Colour '{}' has no hex.", key);
            }

            return ers::ok;
        }


        explicit Palette(ers::DomainView colors) :
            colors(std::move(colors)) {
        }

        std::vector<std::string_view> ids() const {
            return colors.children();
        }

        ers::maybe<std::string_view> hex(std::string_view id) const {
            return colors.find(std::string(id) + ".hex");
        }

        ers::maybe<std::string_view> name(std::string_view id) const {
            return colors.find(std::string(id) + ".name");
        }


        ers::DomainView colors;
    };

    static_assert(ers::DomainAdapter<Palette>);
}


// Keys ------------------------------------------------------------------------

TEST_CASE("localization: keys are made of extension, subfolders and file name") {
    CHECK(*ers::make_localization_key("new_order.txt") == "txt.new_order");
    CHECK(*ers::make_localization_key("sub/deep/footer.html") == "html.sub.deep.footer");
    CHECK(*ers::make_localization_key("a.b.txt") == "txt.a.b");

    CHECK_FALSE(ers::make_localization_key("no_extension"));
}


// Loading ---------------------------------------------------------------------

TEST_CASE("localization: loads every locale and domain") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail", "pages" });
    CHECK(report->kept == 0);
    CHECK(report->removed.empty());
    CHECK(report->rejected.empty());

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

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    CHECK(table["en"]["mail"]["html.sub.footer"] == "<p>Подвал</p>");
    CHECK(table.missing() == std::vector<std::string> { "en: mail/html.sub.footer" });
}

TEST_CASE("localization: unknown names") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());

    CHECK_THROWS(static_cast<void>(table.common()));

    REQUIRE(table.reload(tree.root()));

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

    ers::LocaleTable wrong_fallback({ .fallback_locale = "de" });
    CHECK_FALSE(wrong_fallback.reload(tree.root()));

    ers::LocaleTable table(options());
    CHECK_FALSE(table.reload(tree.root() / "nope"));
    CHECK_FALSE(table.find("ru"));
}


// Common texts ----------------------------------------------------------------

TEST_CASE("localization: the common folder is no locale") {
    TempTree tree;
    fill(tree);
    tree.write("_common/mail/layout.html", "<common/>");
    tree.write("_common/brand/name.txt", "Маленькая Леди");

    ers::LocaleTable table(options());
    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(table.config()->locales == std::vector<std::string> { "en", "ru" });
    CHECK(report->loaded == std::vector<std::string> { "brand", "mail", "pages" });

    // a domain only the common folder has is still seen from every locale
    CHECK(table["en"]["brand"]["txt.name"] == "Маленькая Леди");
    CHECK(table.common()["mail"]["html.layout"] == "<common/>");
}

TEST_CASE("localization: target, then common, then fallback") {
    TempTree tree;
    tree.write("_common/mail/layout.html", "common");
    tree.write("ru/mail/layout.html", "ru");
    tree.write("ru/mail/footer.txt", "ru footer");
    tree.write("en/mail/new_order.txt", "New order");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    CHECK(table["ru"]["mail"]["html.layout"] == "ru");
    CHECK(table["en"]["mail"]["html.layout"] == "common"); // not the fallback's "ru"
    CHECK(table["en"]["mail"]["txt.footer"] == "ru footer");

    CHECK(table.common()["mail"]["html.layout"] == "common");
    CHECK_FALSE(table.common()["mail"].find("txt.footer"));
    CHECK_THROWS(static_cast<void>(table.common()["mail"]["txt.footer"]));

    // common texts are never missing
    CHECK(table.missing() == std::vector<std::string> {
            "en: mail/txt.footer",
        "ru: mail/txt.new_order",
        });
}

TEST_CASE("localization: the common folder's name comes from the options") {
    TempTree tree;
    fill(tree);
    tree.write("shared/mail/layout.html", "shared");

    ers::LocaleTable table({ .fallback_locale = "ru", .common_dir = "shared" });
    REQUIRE(table.reload(tree.root()));

    CHECK(table.config()->locales == std::vector<std::string> { "en", "ru" });
    CHECK(table["en"]["mail"]["html.layout"] == "shared");
}


// Bundles ---------------------------------------------------------------------

TEST_CASE("localization: a bundle inside a domain prefixes its keys") {
    TempTree tree;
    fill(tree);
    tree.write("ru/pages/buttons.l10n.txt", "buy = Купить\ncart.add = В корзину");
    tree.write("ru/pages/checkout/errors.l10n.txt", "phone = Неверный телефон");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    auto pages = table["ru"]["pages"];

    CHECK(pages["buttons.buy"] == "Купить");
    CHECK(pages["buttons.cart.add"] == "В корзину");
    CHECK(pages["checkout.errors.phone"] == "Неверный телефон");

    // the bundle itself is no text
    CHECK_FALSE(pages.find("txt.buttons.l10n"));
}

TEST_CASE("localization: a bundle right under a locale is a domain") {
    TempTree tree;
    fill(tree);
    tree.write("ru/ui.l10n.txt", "buy = Купить");
    tree.write("en/ui/buttons.l10n.txt", "buy = Buy");

    ers::LocaleTable table(options());
    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail", "pages", "ui" });

    CHECK(table["ru"]["ui"]["buy"] == "Купить");
    CHECK(table["en"]["ui"]["buttons.buy"] == "Buy");
}

TEST_CASE("localization: common bundles set locales with '@'") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt",
        "black.hex = #000000\n"
        "black.name@ru = чёрный\n"
        "black.name@en = black\n"
        "milky.hex = #F8F9FA\n"
        "milky.name@ru = молочный\n"
        "brand = Маленькая Леди\n"
        "brand@en = Little Lady\n");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    auto ru = table["ru"]["colors"];
    auto en = table["en"]["colors"];

    CHECK(en["black.hex"] == "#000000");
    CHECK(en["black.name"] == "black");
    CHECK(ru["black.name"] == "чёрный");
    CHECK(en["milky.name"] == "молочный");

    CHECK(ru["brand"] == "Маленькая Леди");
    CHECK(en["brand"] == "Little Lady");
    CHECK(table.common()["colors"]["brand"] == "Маленькая Леди");

    CHECK(table.missing() == std::vector<std::string> {
            "en: colors/milky.name",
        "en: mail/html.sub.footer",
        });
}

TEST_CASE("localization: bundles and files of one domain add up") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt", "black.hex = #000000");
    tree.write("ru/colors.l10n.txt", "black.name = чёрный");
    tree.write("en/colors.l10n.txt", "black.name = black");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    CHECK(table["en"]["colors"]["black.hex"] == "#000000");
    CHECK(table["en"]["colors"]["black.name"] == "black");
    CHECK(table["ru"]["colors"]["black.name"] == "чёрный");
}

TEST_CASE("localization: malformed bundles fail the reload") {
    auto fails = [](std::string_view relative, std::string_view text) {
        TempTree tree;
        fill(tree);
        tree.write(relative, text);

        ers::LocaleTable table(options());
        return !table.reload(tree.root());
    };

    CHECK(fails("ru/ui.l10n.txt", "buy@en = Buy"));         // the path already says ru
    CHECK(fails("_common/ui.l10n.txt", "buy@de = Kaufen")); // no such locale
    CHECK(fails("_common/ui.l10n.txt", "buy@ = Buy"));
    CHECK(fails("_common/ui.l10n.txt", "a@ru.b = x")); // '@' ends the key only
    CHECK(fails("_common/ui.l10n.txt", "a..b = x"));
    CHECK(fails("_common/ui.l10n.txt", "a = x\na = y"));
    CHECK(fails("_common/ui.l10n.txt", "no separator"));

    CHECK_FALSE(fails("_common/ui.l10n.txt", "a = x\na@ru = y"));
}

TEST_CASE("localization: a domain is defined once per locale") {
    TempTree tree;
    fill(tree);
    tree.write("ru/ui/buy.txt", "Купить");
    tree.write("ru/ui.l10n.txt", "buy = Купить");

    ers::LocaleTable table(options());
    CHECK_FALSE(table.reload(tree.root()));

    // a folder in one locale and a file in another are fine
    tree.remove("ru/ui");
    tree.write("en/ui/buy.txt", "Buy");

    CHECK(table.reload(tree.root()));
}


// Children --------------------------------------------------------------------

TEST_CASE("localization: keys and children follow segments") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt",
        "black.hex = #000000\n"
        "black.name = чёрный\n"
        "black-sequins.hex = #000000\n"
        "black-sequins.name = чёрный пайетки\n"
        "blue.hex = #007BFF\n"
        "blue = синий\n");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    auto colors = table["ru"]["colors"];

    CHECK(colors.children() == std::vector<std::string_view> { "black", "black-sequins", "blue" });
    CHECK(colors.children("black") == std::vector<std::string_view> { "hex", "name" });
    CHECK(colors.children("blue") == std::vector<std::string_view> { "hex" });
    CHECK(colors.children("nope").empty());
    CHECK(colors.children("bl").empty());

    const auto black = colors.keys("black");
    CHECK(std::vector(black.begin(), black.end()) == std::vector<std::string_view> { "black.hex", "black.name" });
    CHECK(colors.keys().size() == 6);
    CHECK(colors.keys("nope").empty());

    // a value can have children too
    CHECK(colors["blue"] == "синий");
    CHECK(colors["blue.hex"] == "#007BFF");

    // file keys work the same way
    CHECK(table["ru"]["mail"].children() == std::vector<std::string_view> { "html", "txt" });
    CHECK(table["ru"]["mail"].children("html") == std::vector<std::string_view> { "sub" });
}


// Reloading -------------------------------------------------------------------

TEST_CASE("localization: reload keeps untouched domains as they are") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    const auto mail = domain_of(table, "mail");
    const auto pages = domain_of(table, "pages");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded.empty());
    CHECK(report->kept == 2);

    CHECK(domain_of(table, "mail") == mail);
    CHECK(domain_of(table, "pages") == pages);
}

TEST_CASE("localization: reload keeps domains touched without changes") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    const auto pages = domain_of(table, "pages");

    tree.write("en/pages/about.html", "About");
    tree.touch("en/pages/about.html");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded.empty());
    CHECK(report->kept == 2);
    CHECK(domain_of(table, "pages") == pages);
}

TEST_CASE("localization: reload rebuilds only the changed domain") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    const auto mail = domain_of(table, "mail");
    const auto pages = domain_of(table, "pages");

    const auto old_view = table["en"]["pages"];
    const auto old_text = old_view.share("html.about");

    // Same size on purpose, only the time and the checksum tell it apart.
    tree.write("en/pages/about.html", "Abort");
    tree.touch("en/pages/about.html");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "pages" });
    CHECK(report->kept == 1);

    CHECK(domain_of(table, "mail") == mail);
    CHECK(domain_of(table, "pages") != pages);

    CHECK(table["en"]["pages"]["html.about"] == "Abort");

    // Whoever held the old domain still sees it.
    CHECK(old_view["html.about"] == "About");
    CHECK(*old_text == "About");
}

TEST_CASE("localization: reload sees a changed bundle and the common folder") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt", "black.hex = #000000");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    tree.write("_common/colors.l10n.txt", "black.hex = #111111");
    tree.touch("_common/colors.l10n.txt");

    auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "colors" });
    CHECK(table["ru"]["colors"]["black.hex"] == "#111111");


    tree.write("_common/mail/layout.html", "common");

    report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail" });
    CHECK(table["en"]["mail"]["html.layout"] == "common");
}

TEST_CASE("localization: reload sees a file moved without changing sizes or times") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    // the same bytes under another name, the folder's time put back
    const auto dir = tree.path("ru/pages");
    const auto time = fs::last_write_time(dir);

    fs::rename(tree.path("ru/pages/about.html"), tree.path("ru/pages/team.html"));
    fs::last_write_time(dir, time);

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "pages" });
    CHECK(table["ru"]["pages"]["html.team"] == "О нас");
}

TEST_CASE("localization: reload drops removed domains") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    tree.remove("ru/pages");
    tree.remove("en/pages");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->removed == std::vector<std::string> { "pages" });
    CHECK(report->kept == 1);
    CHECK_FALSE(table["ru"].find("pages"));
}

TEST_CASE("localization: a new locale rebuilds every domain") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    tree.write("de/mail/new_order.txt", "Neue Bestellung");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail", "pages" });
    CHECK(report->kept == 0);

    CHECK(table["de"]["mail"]["txt.new_order"] == "Neue Bestellung");
    CHECK(table["de"]["pages"]["html.about"] == "О нас");
}

TEST_CASE("localization: a failed reload keeps the previous table") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    // Both become "txt.a.b".
    tree.write("ru/mail/a.b.txt", "first");
    tree.write("ru/mail/a/b.txt", "second");

    CHECK_FALSE(table.reload(tree.root()));
    CHECK(table["ru"]["mail"]["txt.new_order"] == "Новый заказ");
    CHECK_FALSE(table["ru"]["mail"].find("txt.a.b"));

    // The failed attempt must not leave stale fingerprints behind.
    tree.remove("ru/mail/a");

    const auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "mail" });
    CHECK(table["ru"]["mail"]["txt.a.b"] == "first");
}


// Validators ------------------------------------------------------------------

TEST_CASE("localization: an invalid domain keeps its previous version") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt", "black.hex = #000000\nblack.name@ru = чёрный");

    ers::LocaleTable table(options());
    REQUIRE(table.attach<Palette>());
    REQUIRE(table.reload(tree.root()));

    const auto colors = domain_of(table, "colors");

    // a broken palette and a fixed page at once
    tree.write("_common/colors.l10n.txt", "black.name@ru = чёрный");
    tree.write("en/pages/about.html", "About us");

    auto report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->loaded == std::vector<std::string> { "pages" });
    REQUIRE(report->rejected.size() == 1);
    CHECK(report->rejected[0].domain == "colors");
    CHECK(report->rejected[0].reason == "Colour 'black' has no hex.");

    CHECK(domain_of(table, "colors") == colors);
    CHECK(table["en"]["pages"]["html.about"] == "About us");


    // it's checked again on every reload until fixed
    report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->rejected.size() == 1);

    tree.write("_common/colors.l10n.txt", "black.hex = #111111\nblack.name@ru = чёрный");
    report = table.reload(tree.root());

    REQUIRE(report);
    CHECK(report->rejected.empty());
    CHECK(report->loaded == std::vector<std::string> { "colors" });
    CHECK(table.adapt<Palette>("en").hex("black") == "#111111"sv);
}

TEST_CASE("localization: an invalid domain fails the first reload and a strict one") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt", "black.name@ru = чёрный");

    ers::LocaleTable table(options());
    REQUIRE(table.attach<Palette>());

    // nothing to keep yet
    CHECK_FALSE(table.reload(tree.root()));


    tree.write("_common/colors.l10n.txt", "black.hex = #000000");

    ers::LocaleTable strict(options(ers::EInvalidDomain::FailReload));
    REQUIRE(strict.attach<Palette>());
    REQUIRE(strict.reload(tree.root()));

    tree.write("_common/colors.l10n.txt", "black.name@ru = чёрный");
    tree.write("en/pages/about.html", "About us");

    CHECK_FALSE(strict.reload(tree.root()));
    CHECK(strict["en"]["pages"]["html.about"] == "About");
}

TEST_CASE("localization: a validator added late checks the current domain") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt", "black.name@ru = чёрный");

    ers::LocaleTable table(options());
    REQUIRE(table.reload(tree.root()));

    CHECK_FALSE(table.attach<Palette>());

    // the domain didn't change, but it's rebuilt and checked again
    const auto report = table.reload(tree.root());

    REQUIRE(report);
    REQUIRE(report->rejected.size() == 1);
    CHECK(report->rejected[0].domain == "colors");
}

TEST_CASE("localization: validators by name, exceptions are errors") {
    TempTree tree;
    fill(tree);

    ers::LocaleTable table(options());

    REQUIRE(table.add_validator("mail", [](const ers::DomainPtr& mail) -> ers::Status {
        if (!mail->find("txt.new_order", mail->config().fallback_locale_id))
        return ers::make_error("No new order mail.");
        return ers::ok;
    }));

    REQUIRE(table.add_validator("pages", [](const ers::DomainPtr&) -> ers::Status {
        throw std::runtime_error("boom");
    }));

    CHECK_FALSE(table.reload(tree.root()));

    ers::LocaleTable other(options());
    REQUIRE(other.add_validator("mail", [](const ers::DomainPtr&) { return ers::Status(ers::ok); }));
    CHECK(other.reload(tree.root()));
}

TEST_CASE("localization: adapters read their domain") {
    TempTree tree;
    fill(tree);
    tree.write("_common/colors.l10n.txt",
        "black.hex = #000000\nblack.name@ru = чёрный\nblack.name@en = black\n"
        "milky.hex = #F8F9FA\nmilky.name@ru = молочный\n");

    ers::LocaleTable table(options());
    REQUIRE(table.attach<Palette>());
    REQUIRE(table.reload(tree.root()));

    auto en = table.adapt<Palette>("en");

    CHECK(en.ids() == std::vector<std::string_view> { "black", "milky" });
    CHECK(en.name("black") == "black"sv);
    CHECK(en.name("milky") == "молочный"sv);
    CHECK(en.hex("milky") == "#F8F9FA"sv);

    CHECK_THROWS(static_cast<void>(table.adapt<Palette>("de")));
}
