/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split_unowned.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgoulard <bgoulard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/16 16:19:01 by bgoulard          #+#    #+#             */
/*   Updated: 2024/02/16 16:37:19 by bgoulard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

static int nb_words(char *str, char delim)
{
	int i;
	int nb;
	int in_word;

	i = 0;
	nb = 0;
	in_word = 0;
	while (str[i])
	{
		if (str[i] == delim && in_word)
		{
			nb++;
			in_word = 0;
		}
		else if (str[i] != delim)
			in_word = 1;
		i++;
	}
	return (nb + in_word);
}

char **split_unowned(char *str, char delim)
{
	int		nbs;
	char	**res;

	nbs = nb_words(str, delim);
	res = malloc(sizeof(char *) * (nbs + 1));
	if (!res)
		return (NULL);
	nbs = 0;
	while (*str)
	{
		while (*str == delim)
			str++;
		if (!*str)
			break ;
		res[nbs++] = str;
		while (*str && *str != delim)
			str++;
		if (*str)
			*str++ = 0;
	}
	res[nbs] = NULL;
	return (res);
}

int main()
{
	char str[] = "  Hello World             this is a test   ";
	char **res = split_unowned(str, ' ');
	int i = 0;

	for (i = 0; res[i]; i++)
	{
		printf("%s\n", res[i]);
	}
	free(res);
	return (0);
}