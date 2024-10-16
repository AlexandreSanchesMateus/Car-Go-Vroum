#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

class Command 
{
public:
	Command();

	enum Action : std::uint8_t
	{
		Purge,
		Kick,
		Close,
		Easter
	};

	struct Report
	{
		Action action;
		int params = 0;
	};

	std::optional<Report> HandleEvent();
	void ClearLastPrompt();
	void RecoverLastPrompt();

private:
	const std::unordered_map<std::string, Action> m_commands = {
		{"purge", Action::Purge}, {"kick", Action::Kick}, {"close", Action::Close}, {"jerome", Action::Easter}
	};

	std::string m_prompt;

	bool is_number(const std::string& s) const;
};