/*
 * main.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: nbingham
 */

#include <common/standard.h>
#include <parse/parse.h>
#include <parse/default/block_comment.h>
#include <parse/default/line_comment.h>
#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>
#include <parse_prs/production_rule_set.h>
#include <prs/production_rule.h>
#include <prs/bubble.h>
#include <interpret_prs/import.h>
#include <interpret_prs/export.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <ucs/variable.h>

#ifdef GRAPHVIZ_SUPPORTED
namespace graphviz
{
	#include <graphviz/cgraph.h>
	#include <graphviz/gvc.h>
}
#endif

void print_help()
{
	printf("Usage: bubble [options] file...\n");
	printf("A simulation environment for PRS processes.\n");
	printf("\nSupported file formats:\n");
	printf(" *.prs           Load a production rule set\n");
	printf("\nGeneral Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -v,--verbose   Display verbose messages\n");
	printf(" -d,--debug     Display internal debugging messages\n");
	printf("\nConversion Options:\n");
	printf(" -o <filename>  Specify the output filename for the reshuffled prs\n");
	printf(" -r <format>    Render each step in the bubble reshuffling\n");
	printf("                formats other than 'dot' are passed to graphviz dot for rendering\n");
}

void print_version()
{
	printf("bubble 1.0.0\n");
	printf("Copyright (C) 2023 Ned Bingham.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

void save_bubble(string fmt, int step, const prs::bubble &bub, const ucs::variable_set &vars)
{
	string dot = export_bubble(bub, vars).to_string();
	string filename = "step" + std::to_string(step) + "." + fmt;
	
	if (fmt == "dot") {
		FILE *file = fopen(filename.c_str(), "w");
		fprintf(file, "%s\n", dot.c_str());
		fclose(file);
	} else {
#ifdef GRAPHVIZ_SUPPORTED
		graphviz::Agraph_t* G = graphviz::agmemread(dot.c_str());
		graphviz::GVC_t* gvc = graphviz::gvContext();
		graphviz::gvLayout(gvc, G, "dot");
		graphviz::gvRenderFilename(gvc, G, fmt.c_str(), filename.c_str());
		graphviz::gvFreeLayout(gvc, G);
		graphviz::agclose(G);
		graphviz::gvFreeContext(gvc);
#else
		string tfilename = filename.substr(0, filename.find_last_of("."));
		FILE *temp = NULL;
		int num = 0;
		for (; temp == NULL; num++)
			temp = fopen((tfilename + (num > 0 ? to_string(num) : "") + ".dot").c_str(), "wx");
		num--;
		tfilename += (num > 0 ? to_string(num) : "") + ".dot";

		fprintf(temp, "%s\n", dot.c_str());
		fclose(temp);

		if (system(("dot -T" + fmt + " " + tfilename + " > " + filename).c_str()) != 0)
			error("", "Graphviz DOT not supported", __FILE__, __LINE__);
		else if (system(("rm -f " + tfilename).c_str()) != 0)
			warning("", "Temporary files not cleaned up", __FILE__, __LINE__);
#endif
	}
}

int main(int argc, char **argv)
{
	configuration config;
	config.set_working_directory(argv[0]);
	tokenizer tokens;
	parse_prs::production_rule_set::register_syntax(tokens);
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);

	bool render_steps = false;
	string ofilename = "bubble.prs";
	string oformat = "png";

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "--help" || arg == "-h")			// Help
		{
			print_help();
			return 0;
		}
		else if (arg == "--version")	// Version Information
		{
			print_version();
			return 0;
		}
		else if (arg == "--verbose" || arg == "-v")
			set_verbose(true);
		else if (arg == "--debug" || arg == "-d")
			set_debug(true);
		else if (arg == "--render" || arg == "-r") {
			render_steps = true;
			i++;
			if (i < argc) {
				oformat = argv[i];
			} else {
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		} else if (arg == "-o") {
			i++;
			if (i < argc) {
				ofilename = argv[i];
			} else {
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		} else {
			string filename = argv[i];
			size_t dot = filename.find_last_of(".");
			string format = "";
			if (dot != string::npos)
				format = filename.substr(dot+1);
			if (format == "prs")
				config.load(tokens, filename, "");
			else
				printf("unrecognized file format '%s'\n", format.c_str());
		}
	}

	if (is_clean() && tokens.segments.size() > 0)
	{
		prs::production_rule_set pr;
		ucs::variable_set v;

		tokens.increment(false);
		tokens.expect<parse_prs::production_rule_set>();
		if (tokens.decrement(__FILE__, __LINE__))
		{
			parse_prs::production_rule_set syntax(tokens);
			pr = import_production_rule_set(syntax, v, 0, &tokens, true);
		}
		pr.post_process(v);

		prs::bubble bub;
		bub.load_prs(pr, v);

		int step = 0;
		if (render_steps) {
			save_bubble(oformat, step++, bub, v);
		}
		for (auto i = bub.net.begin(); i != bub.net.end(); i++) {
			pair<int, bool> result = bub.step(i);
			if (render_steps && result.second) {
				save_bubble(oformat, step++, bub, v);
			}
		}
	
		bub.save_prs(&pr, v);
		string rules = export_production_rule_set(pr, v).to_string();
		FILE *file = fopen(ofilename.c_str(), "w");
		fprintf(file, "%s\n", rules.c_str());
		fclose(file);
	}

	complete();
	return is_clean();
}
