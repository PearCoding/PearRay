#include "registry/Registry.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Registry)
PR_TEST("Parameter")
{
	Parameter param(true);
	PR_CHECK_TRUE(param.type() == PT_Boolean);
}

PR_TEST("URI /test/foo.txt")
{
	const URI uri("/test/foo.txt");
	PR_CHECK_EQ(uri.protocol(), "");
	PR_CHECK_EQ(uri.host(), "");
	PR_CHECK_EQ(uri.port(), "");
	PR_CHECK_EQ(uri.path(), "/test/foo.txt");
	PR_CHECK_EQ(uri.query(), "");
	PR_CHECK_EQ(uri.fragment(), "");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_TRUE(uri.isAbsolute());
	PR_CHECK_FALSE(uri.isExternal());
}

PR_TEST("URI ./test/foo.txt#section1")
{
	const URI uri("./test/foo.txt#section1");
	PR_CHECK_EQ(uri.protocol(), "");
	PR_CHECK_EQ(uri.host(), "");
	PR_CHECK_EQ(uri.port(), "");
	PR_CHECK_EQ(uri.path(), "./test/foo.txt");
	PR_CHECK_EQ(uri.query(), "");
	PR_CHECK_EQ(uri.fragment(), "section1");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_FALSE(uri.isAbsolute());
	PR_CHECK_FALSE(uri.isExternal());
}

PR_TEST("URI https://www.google.com/search?q=PearRay")
{
	const URI uri("https://www.google.com/search?q=PearRay");
	PR_CHECK_EQ(uri.protocol(), "https");
	PR_CHECK_EQ(uri.host(), "www.google.com");
	PR_CHECK_EQ(uri.port(), "");
	PR_CHECK_EQ(uri.path(), "/search");
	PR_CHECK_EQ(uri.query(), "q=PearRay");
	PR_CHECK_EQ(uri.fragment(), "");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_TRUE(uri.isExternal());
}

PR_TEST("URI telnet://192.0.2.16:80/")
{
	const URI uri("telnet://192.0.2.16:80/");
	PR_CHECK_EQ(uri.protocol(), "telnet");
	PR_CHECK_EQ(uri.host(), "192.0.2.16");
	PR_CHECK_EQ(uri.port(), "80");
	PR_CHECK_EQ(uri.path(), "/");
	PR_CHECK_EQ(uri.query(), "");
	PR_CHECK_EQ(uri.fragment(), "");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_TRUE(uri.isExternal());
}

PR_TEST("URI http://nobody@example.org:8080/cgi-bin/script.php?action=submit&pageid=86392001#section_2")
{
	const URI uri("http://nobody@example.org:8080/cgi-bin/script.php?action=submit&pageid=86392001#section_2");
	PR_CHECK_EQ(uri.protocol(), "http");
	PR_CHECK_EQ(uri.host(), "nobody@example.org");
	PR_CHECK_EQ(uri.port(), "8080");
	PR_CHECK_EQ(uri.path(), "/cgi-bin/script.php");
	PR_CHECK_EQ(uri.query(), "action=submit&pageid=86392001");
	PR_CHECK_EQ(uri.fragment(), "section_2");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_TRUE(uri.isExternal());
}

PR_TEST("URI /root + ./test")
{
	const URI base("/root");
	const URI p("./test");

	URI uri = URI::makeAbsolute(p, base);

	PR_CHECK_EQ(base.path(), "/root");
	PR_CHECK_EQ(p.path(), "./test");

	PR_CHECK_EQ(uri.protocol(), "");
	PR_CHECK_EQ(uri.host(), "");
	PR_CHECK_EQ(uri.port(), "");
	PR_CHECK_EQ(uri.path(), "/root/test");
	PR_CHECK_EQ(uri.query(), "");
	PR_CHECK_EQ(uri.fragment(), "");
	PR_CHECK_TRUE(uri.isValid());
	PR_CHECK_FALSE(uri.isExternal());
}

PR_TEST("Registry SET/GET")
{
	Registry registry;
	const URI uri("/test");
	const URI uri2("/test2");

	PR_CHECK_FALSE(registry.exists(uri));

	registry.set(uri, Parameter(42));
	PR_CHECK_TRUE(registry.exists(uri));
	PR_CHECK_EQ(registry.get(uri, Parameter(-1)).getInteger(), 42);
	PR_CHECK_EQ(registry.get(uri2, Parameter(-1)).getInteger(), -1);
}

PR_TEST("Registry Material SET/GET")
{
	Registry registry;
	const uint32 matID = 0xdeadbeef;
	const URI uri("test");
	const URI uri2("test2");

	PR_CHECK_FALSE(registry.existsMaterial(matID, uri));

	registry.setMaterial(matID, uri, Parameter(42));
	PR_CHECK_TRUE(registry.existsMaterial(matID, uri));
	PR_CHECK_EQ(registry.getMaterial(matID, uri, Parameter(-1)).getInteger(), 42);
	PR_CHECK_EQ(registry.getMaterial(matID, uri2, Parameter(-1)).getInteger(), -1);
}

PR_TEST("Registry Material SET/GET GLOBAL")
{
	Registry registry;
	const uint32 matID = 0xdeadbeef;
	const URI uri("test");
	const URI uri2("test2");

	PR_CHECK_FALSE(registry.existsMaterial(matID, uri));
	PR_CHECK_FALSE(registry.existsGlobal(uri));

	registry.setGlobal(uri, Parameter(81));
	PR_CHECK_TRUE(registry.existsMaterial(matID, uri));
	PR_CHECK_EQ(registry.getMaterial(matID, uri, Parameter(-1)).getInteger(), 81);
	PR_CHECK_EQ(registry.getMaterial(matID, uri2, Parameter(-1)).getInteger(), -1);

	registry.setMaterial(matID, uri, Parameter(42));
	PR_CHECK_TRUE(registry.existsMaterial(matID, uri));
	PR_CHECK_EQ(registry.getMaterial(matID, uri, Parameter(-1)).getInteger(), 42);
	PR_CHECK_EQ(registry.getMaterial(matID, uri2, Parameter(-1)).getInteger(), -1);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Registry);
PRT_END_MAIN