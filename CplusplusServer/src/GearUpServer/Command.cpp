#include "GearUpServer/Command.hpp"
#include <conio.h>
#include <fmt/core.h>
#include <fmt/color.h>

Command::Command()
{
	fmt::println("< ======================================== >");
	fmt::println("\nList of commands :");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    purge");
	fmt::println("             Ejects all players not initialized");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    kick [id]");
	fmt::println("         Ejects a particular player with the corresponding id");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    close");
	fmt::println("             Closes the server\n");

	fmt::print("> ");
}

std::optional<Command::Report> Command::HandleEvent()
{
	std::optional<Report> report = std::nullopt;
	if (_kbhit())
	{
		char c = _getch();
		if (c < 0)
			return std::nullopt;

		fmt::print("{}", c);

		if (c == '\b')
		{
			// backspace key
			if (!m_prompt.empty())
			{
				fmt::print(" \b");
				m_prompt.pop_back();
			}
			else
				fmt::print(" ");
		}
		else if (c == '\r')
		{
			// enter key
			if (m_prompt.empty())
			{
				fmt::print("> ");
				return std::nullopt;
			}

			fmt::println("");

			std::unordered_map<std::string, Action>::const_iterator it;
			std::size_t spacer = m_prompt.find(' ');
			if (spacer != std::string::npos)
			{
				std::string str = m_prompt.substr(0, spacer);
				it = m_commands.find(str);
			}
			else
				it = m_commands.find(m_prompt);

			if (it == m_commands.end()) {
				fmt::print(stderr, fg(fmt::color::yellow), "[SERV REPLY]");
				fmt::println(" No command find for : {}", m_prompt);
			}
			else
			{
				report = Report();
				report->action = it->second;

				switch (it->second)
				{
				case Action::Close:
				case Action::Purge:
					break;

				case Action::Kick:
					if (spacer != std::string::npos)
					{
						std::string param = m_prompt.substr(spacer + 1);
						if (is_number(param))
						{
							report->params = std::stoi(param);
						}
						else
						{
							fmt::print(stderr, fg(fmt::color::yellow), "[SERV REPLY]");
							fmt::println(" Invalid params for kick command. Must be numerical");
							fmt::print("> ");
							m_prompt.clear();
							return std::nullopt;
						}
					}
					else
					{
						fmt::print(stderr, fg(fmt::color::yellow), "[SERV REPLY]");
						fmt::println(" No params for kick command");
						fmt::print("> ");
						m_prompt.clear();
						return std::nullopt;
					}

					break;
				case Action::Easter:
					fmt::println("                              @@%@         @@@@@@@@@@@@@@@@@@");
					fmt::println("                               @@@@@@@@@@%%%################%%@@");
					fmt::println("                           *#@@@@@%%###########################%@");
					fmt::println("                         @@%###################################%%@");
					fmt::println("                      @@%%####################################%%@@  @@@@");
					fmt::println("                    @@%##########%%#########################%#%%%%%@%@@@");
					fmt::println("                   @%####%%%%%%%%@%%#######################%@@@@%@@@%@@@");
					fmt::println("                 @%######%%@@@@@%%##########################%@@@@%@@@%@@%%@@@");
					fmt::println("              @@%#########%%@@%%###########################%%++#%@@@@@@@@@%@@");
					fmt::println("             @@@%############################%@%%########%%*====++##%%@@@@%%@@@@");
					fmt::println("               %@########################%@@@@%%######%@%*+++*#%@%+===+*%@%%%@@@");
					fmt::println("               @%######################%@@@%%@@%####%@%%%@@@@@@@@*=-----=@%%@@@@");
					fmt::println("              @%########%%%##########%@@@%%%@@%##%%%*#@@%%%%%%@%*--------%@%%%@@");
					fmt::println("             @%%#######%%@@%######%@@@%%%%@@@@%%%*+===*%@@@@@%*=---------=@%%%@@");
					fmt::println("           @@%%#######%%@@%####%@@@%%%@@@%%##*+===-------===--------------+%@@@@");
					fmt::println("          @@%%%%%##%%%%@@%##%%%#%%%%%##+=====-------------------------------+@@@@");
					fmt::println("         @@@@@@@%#%%%%%@@%%@#+==========------------------------------------+@");
					fmt::println("             @@%%%%%%%%%@@@@#+=====-----------------------------------------+@");
					fmt::println("            @@%%%%%%%%%%@@@@#+===---------------------------=++=------------+@");
					fmt::println("            @@%%%%%%%%%%%%@@*==--------------=+++=--------=#@@@@#=----------+@");
					fmt::println("           @@@%%%%%%%%%%%%@%*=-------------=#@@@@%=-------=@@@@@%+----------=@");
					fmt::println("           @@@%%%%%%%%%%%%@#==-------------=%@@@@@+-------=+@@@@*-----------=%");
					fmt::println("            @@%%%%%%%%%%%@%*=---------------+%@@@*=----------------------+=-=*@");
					fmt::println("            @@@%%%%%%%%%%@%+=---------------------------------------=+*%%#%=-+@");
					fmt::println("           @@@@@%%%%@%%%%@#==-----------------------------==*#%%%#*+==-------=#@");
					fmt::println("              @@%%%%@@%%@@#==----------------==++*#%%%#**+====----------------+@");
					fmt::println("               @@%%%@@@@%@#+=-----*%####**+++======---------------------------=%");
					fmt::println("               %@%@@@@#%@@@#+==---=-------------------------------------------+@");
					fmt::println("             @@@%%@@@@%+#%@@%%*=---------------------------------------------=#@");
					fmt::println("              @@@@#**%@+=*%**##=---------------------------------------------+@");
					fmt::println("                @#====+===--++*#+-------------------------------------------=%@");
					fmt::println("                @+=#=====+*####*+=------------------------------------------+@");
					fmt::println("                %%=#######*+=-=++=----------------------------------+=--++-=%@");
					fmt::println("                 #%+--=------------==-----=+=-+*=-------------------+%+-=@++@");
					fmt::println("                   @%*+==----==++#%##%#+=*%+=*%+-++=-----------------+%+-+@%");
					fmt::println("                      %@@@%%@@@@ @#+==+*%%#+*%+-*%+----------------+*=#%+#@");
					fmt::println("                                  #@*=====+#%#+*%+-----------------=%*#@@");
					fmt::println("                                     @#====================++=======+%                 @@@@@@@@@@@@@");
					fmt::println("                                      @*--------===========-----====+%            @@@@%%##########%%@@@");
					fmt::println("                                       %------------------------*%+==#%      @@@@%%###################%@@@");
					fmt::println("           @@@@@@%%%%%%@@@@@@          @=----------------------+%*====#@@@@@%###########################%@@");
					fmt::println("        @@%%################%%@@       @------------------------=======+@%################################%@");
					fmt::println("      @@%#######################%%@@ @@%---------------------------====%@##################################%@");
					fmt::println("    @@%#############################%%#%%+---------------------------=*@%###################################%");
					fmt::println("   @@###################################%%@#+=----------------------=#@%#####################################");
					fmt::println("  @@#######################################%%%%@%#+=--------------+%@%#######################################");
					fmt::println(" @@%############################################%%%%%%%%%%#####%%%%%#########################################");
					fmt::println(" @%######################################################%%%%%%##############################################");
					fmt::println("@%###########################################################################################################");
					fmt::println("@%###########################################################################################################");
					break;
				}
			}

			fmt::print("> ");
			m_prompt.clear();
		}
		else
			m_prompt.push_back(c);
	}

	return report;
}

void Command::ClearLastPrompt() const
{
	fmt::print("\r{}\r", std::string(m_prompt.length() + 2, ' '));
}

void Command::RecoverLastPrompt() const
{
	fmt::print("> {}", m_prompt);
}

bool Command::is_number(const std::string& str) const
{
	std::string::const_iterator it = str.begin();
	while (it != str.end() && std::isdigit(*it))
		++it;

	return !str.empty() && it == str.end();
}