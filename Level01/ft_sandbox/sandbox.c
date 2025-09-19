/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sandbox.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carlopez <carlopez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 12:46:32 by carlopez          #+#    #+#             */
/*   Updated: 2025/09/19 16:37:15 by carlopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

static pid_t child_pid;

void    alarm_handler(int sig)
{
    (void)sig;
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
    pid_t pid;
    struct sigaction sa;
    int status;
    int sig;

    //Configuramos sigaction para SIGALRM
    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    //Creamos proceso hijo que ejecuta f()
    pid = fork();
    if (pid == -1)
        return (-1);
    if (pid == 0)
    {
        f();
        exit(0);
    }
    
    //En el padre, inicia la alarma que lanzara SIGALRM pasado el timeout
    child_pid = pid;
    alarm(timeout);

    //Espera a que acabe el hijo: si waitpid devuelve
    //-1 y ERRNO==EINTR, ha sido interrumpido por SIGALRM
    if (waitpid(pid, &status, 0) == -1)
    {
        if (errno == EINTR)
        {
            //un signal interrumpio waitpid
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            if (verbose)
                printf("Bad function: timed out after %d seconds\n", timeout);
            return (0);
        }
        return (-1);
    }
    alarm(0); //Para cancelar la alarma

    //Una vez aqui, waitpid devolvio bien status y cntiene la info de salida
    
    //caso de que ha terminado  normal
    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0)
        {
            if (verbose)
                printf("Nice function!\n");
            return (1);
        }
        else
        {
            if (verbose)
                printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
            return (0);
        }
    }

    //caso termino por senal
    if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        if (verbose)
            printf("Bad function: %s\n", strsignal(sig));
        return (0);
    }
    return (-1);
}

//A partir de aqui funciones para probar el main:
void nice_function(void)
 {
    return;
 }


void bad_function_exit_code(void)
{
    // Esta función termina con código de error
    exit(1);
}

void bad_function_segfault(void)
{
    // Esta función causa segmentation fault
    int *ptr = NULL;
    *ptr = 42;
}

void bad_function_timeout(void)
{
    // Esta función nunca termina
    while (1) {}
}

void bad_function_abort(void)
{
    // Esta función llama abort()
    abort();
}

// Helper to check sandbox result and print outcome
void check_result(const char *test_name, int expected, int actual) {
	if (expected == actual)
		printf("[PASS] %s (expected: %d, got: %d)\n", test_name, expected, actual);
	else
		printf("[FAIL] %s (expected: %d, got: %d)\n", test_name, expected, actual);
}

// Additional test functions
/*
void bad_function_divzero(void) {
	int x = 1/0;
	(void)x;
}*/

void bad_function_sigfpe(void) {
	raise(SIGFPE);
}

void bad_function_sigterm(void) {
	raise(SIGTERM);
}

void bad_function_sigkill(void) {
	raise(SIGKILL);
}

void nice_function_sleep(void) {
	sleep(1);
}

int main(void)
{
	int result;

	// Test: nice function
	result = sandbox(nice_function, 5, false);
	check_result("nice_function", 1, result);

	// Test: nice function with sleep
	result = sandbox(nice_function_sleep, 5, false);
	check_result("nice_function_sleep", 1, result);

	// Test: bad function exit code
	result = sandbox(bad_function_exit_code, 5, false);
	check_result("bad_function_exit_code", 0, result);

	// Test: bad function segfault
	result = sandbox(bad_function_segfault, 5, false);
	check_result("bad_function_segfault", 0, result);

	// Test: bad function timeout
	result = sandbox(bad_function_timeout, 2, false);
	check_result("bad_function_timeout", 0, result);

	// Test: bad function abort
	result = sandbox(bad_function_abort, 5, false);
	check_result("bad_function_abort", 0, result);

	// Test: bad function division by zero
	//result = sandbox(bad_function_divzero, 5, false);
	//check_result("bad_function_divzero", 0, result);

	// Test: bad function SIGFPE
	result = sandbox(bad_function_sigfpe, 5, false);
	check_result("bad_function_sigfpe", 0, result);

	// Test: bad function SIGTERM
	result = sandbox(bad_function_sigterm, 5, false);
	check_result("bad_function_sigterm", 0, result);

	// Test: bad function SIGKILL
	result = sandbox(bad_function_sigkill, 5, false);
	check_result("bad_function_sigkill", 0, result);

	// Test: zero timeout (should always timeout)
	result = sandbox(nice_function_sleep, 0, false);
	check_result("nice_function_sleep_zero_timeout", 1, result);

	// Test: NULL function pointer (should not crash, but sandbox will fork and exit)
	result = sandbox(NULL, 5, false);
	check_result("null_function_pointer", 0, result);

	return 0;
}

