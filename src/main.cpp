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
}

void print_version()
{
	printf("bubble 1.0.0\n");
	printf("Copyright (C) 2023 Ned Bingham.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	configuration config;
	config.set_working_directory(argv[0]);
	tokenizer tokens;
	parse_prs::production_rule_set::register_syntax(tokens);
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	vector<prs::term_index> steps;

	bool labels = false;

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
		else
		{
			string filename = argv[i];
			int dot = filename.find_last_of(".");
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
		string dot = export_bubble(bub, v).to_string();
		FILE *file = fopen("before.dot", "w");
		fprintf(file, "%s\n", dot.c_str());
		fclose(file);
		
		bub.reshuffle(v);
	
		dot = export_bubble(bub, v).to_string();
		file = fopen("after.dot", "w");
		fprintf(file, "%s\n", dot.c_str());
		fclose(file);

		bub.save_prs(&pr, v);
		string rules = export_production_rule_set(pr, v).to_string();
		file = fopen("bubble.prs", "w");
		fprintf(file, "%s\n", rules.c_str());
		fclose(file);
	}

	complete();
	return is_clean();
}
