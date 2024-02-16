/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split_quotes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgoulard <bgoulard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/16 14:43:30 by bgoulard          #+#    #+#             */
/*   Updated: 2024/02/16 16:17:42 by bgoulard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int	nb_words_block(char *str, char delim)
{
	int	i;
	int	nb;
	int	flag;

	i = 0;
	nb = 0;
	flag = 0;
	while (str[i])
	{
		if (str[i] == '\'' && (flag == 0 || flag == 1))
			flag = !flag;
		else if (str[i] == '\"' && (flag == 0 || flag == 2))
			flag = (flag == 0) * 2;
		if (str[i] == delim && !flag)
			nb++;
		i++;
	}
	return (nb + 1);
}

int	split_sz(char *str, int *offset, char delim)
{
	int	si_quotes;
	int	db_quotes;
	int	size;

	si_quotes = 0;
	db_quotes = 0;
	size = 0;
	while (str[*offset] == delim && str[*offset])
		(*offset)++;
	while (str[*offset + size])
	{
		if (str[*offset + size] == '\'' && !db_quotes)
			si_quotes = !si_quotes;
		if (str[*offset + size] == '"' && !si_quotes)
			db_quotes = !db_quotes;
		if (str[*offset + size] == delim && !si_quotes && !db_quotes)
			break ;
		size++;
	}
	return (size);
}

// split string by delim but keeps char in " and ' as single blocks
// note : the string are copied in the array
char	**split_block(char *str, char delim)
{
	const int	nb = nb_words_block(str, delim);
	char		**res;
	int			i;
	int			wb_size;
	int			j;

	i = 0;
	j = 0;
	res = malloc(sizeof(char *) * (nb + 1));
	if (!res)
		return (NULL);
	res[nb] = NULL;
	while (j != nb)
	{
		wb_size = split_sz(str, &i, delim);
		res[j] = malloc(sizeof(char) * (wb_size + 1));
		memcpy(res[j], str + i, wb_size); // ft_memcpy
		res[j++][wb_size] = '\0';
		i += wb_size + 1;
	}
	return (res);
}
