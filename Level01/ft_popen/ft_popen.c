/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_popen.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlopez <carlopez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:33:28 by carlopez          #+#    #+#             */
/*   Updated: 2025/09/17 18:34:52 by carlopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int ft_popen(const char *file, char *const argv[], char type)
{
    int fds[2];
    int pid;
    
    if (!file || !argv || (type != 'r' && type != 'w'))
        return (1);
    if (pipe(fds) == -1)
        return (1);
    pid = fork();
    if (pid == -1)
    {
        close(fds[0]);
        close(fds[1]);
        return (1);
    }
    if (pid == 0)
    {
        if (type == 'r')
        {
            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
            close(fds[1]);
        }
        else
        {
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
        }
        execvp(file, argv);
        exit(-1);
    }
    else
    {
        if (type == 'r')
        {
            close(fds[1]);
            return (fds[0]);
        }
        else
        {
            close(fds[0]);
            return (fds[1]);
        }
    }
    return (-1);
}

int main(void)
{
    char buffer[1024];
    int fd, n;

    // ================================
    // Ejemplo 1: type == 'r'
    // ================================
    printf("=== Test 'r': ft_popen(\"ls\") ===\n");

    char *args_ls[] = {"ls", NULL};
    fd = ft_popen("ls", args_ls, 'r');
    if (fd == -1)
    {
        perror("ft_popen");
        return (1);
    }

    while ((n = read(fd, buffer, sizeof(buffer)-1)) > 0)
    {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    close(fd);

    // ================================
    // Ejemplo 2: type == 'w'
    // ================================
    printf("\n=== Test 'w': ft_popen(\"cat\") ===\n");

    char *args_cat[] = {"cat", NULL};
    fd = ft_popen("cat", args_cat, 'w');
    if (fd == -1)
    {
        perror("ft_popen");
        return (1);
    }

    char *msg = "Hola desde el padre!\nEsto se envía al hijo (cat).\n";
    write(fd, msg, strlen(msg));
    close(fd);

    return (0);
}