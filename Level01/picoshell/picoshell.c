/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   picoshell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlopez <carlopez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 10:45:01 by carlopez          #+#    #+#             */
/*   Updated: 2025/09/17 11:35:22 by carlopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int picoshell(char **cmd[])
{
    int fds[2];
    int prev_fd;
    int i;
    int pid;

    prev_fd = -1;
    i = 0;
    if (!cmd || !cmd[0])
        return (1);
    while (cmd[i])
    {
        //Si hay otro comando despues creo la pipe para conectar los
        if (cmd[i + 1] && pipe(fds) == -1)
            return (1);
        //hago el hijo
        pid = fork();
        if (pid == -1)
            return (1);
        if (pid == 0)
        {
            // si estoy en el hijo y existe un comando previo leo de ahi
            if (prev_fd != -1)
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if (cmd[i + 1])
            {
                //si hay comando detras escribe alli
                close(fds[0]);
                dup2(fds[1], STDOUT_FILENO);
                close(fds[1]);
            }
            //ejecuta comando, si falla hijo sale con 1
            execvp(cmd[i][0], cmd[i]);
            exit(1);
        }
        //si esta en el padre y hay previo cierra el fd, si hay siguiente, se guarda
        //el nuevo previo
        if (prev_fd != -1)
            close(prev_fd);
        if (cmd[i + 1])
        {
            close(fds[1]);
            prev_fd = fds[0];
        }
        i++;
    }
    //devuelve -1 cuando ha esperado a todos los hijos
    while (wait(NULL) > 0)
    ;
        return (0);
}

int main() 
{
    char *ls[] = {"ls", NULL};
    char *wc[] = {"wc", "-l", NULL};
    char **cmds[] = {ls, wc, NULL};
    return (picoshell(cmds));
}