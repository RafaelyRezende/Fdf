/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unit_tester.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluis-ya <rluis-ya@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 09:49:07 by rluis-ya          #+#    #+#             */
/*   Updated: 2025/08/02 19:22:09 by rluis-ya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "iso_fdf.h"

static
int	ft_error(const char* msg)
{
	printf("Error %s...", msg);
	return (-1);
}

static
int	ft_expose(t_env *this)
{
	mlx_put_image_to_window(this->mlx, this->mlx_win, this->window.img, 0, 0);
	return (0);
}

int	main(int argc, char **argv)
{
	t_env	this;

	if (argc != 2)
		return (ft_error("number of arguments"));
	this.mlx = mlx_init();
	if (init_map(argv[1], &this.map))
		return (ft_clean_exit(&this));
	this.mlx_win = mlx_new_window(this.mlx, SCREEN_W, SCREEN_H, NAME);
	this.window.img = mlx_new_image(this.mlx, SCREEN_W, SCREEN_H);
	this.window.addr = mlx_get_data_addr(this.window.img, &this.window.bits_per_pixel, &this.window.line_length, &this.window.endian);
	if (!this.mlx_win || !this.window.addr)
		ft_clean_exit(&this);
	ft_get_center(&this);
	mlx_loop_hook(this.mlx, ft_display_img, &this);
	mlx_key_hook(this.mlx_win, ft_keypress, &this);
	mlx_hook(this.mlx_win, 2, 1L<<0, ft_keypress, &this);
	mlx_hook(this.mlx_win, 17, 1L<<17, ft_clean_exit, &this);
	mlx_expose_hook(this.mlx_win, ft_expose, &this);
	ft_display_img(&this);
	return (0);
}
/*
static
void	print_quat(t_map *map)
{
		printf("%f,%f,%f,%f\n", map->q.w, map->q.x, map->q.y, map->q.z);
}
static
void	print_grid(t_map *map)
{
	unsigned int i = 0;
	while (i < map->size)
	{
		printf("%f,%f,%f\n", map->grid[i].x, map->grid[i].y, map->grid[i].z);
		i++;
	}
}
*/
