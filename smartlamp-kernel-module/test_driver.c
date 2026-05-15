#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Mock sysfs do SmartLamp para testar o daemon de brilho");
MODULE_LICENSE("GPL");

static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff);
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count);

static struct kobj_attribute led_attribute = __ATTR(led, 0664, attr_show, attr_store);
static struct kobj_attribute ldr_attribute = __ATTR(ldr, 0664, attr_show, attr_store);
static struct kobj_attribute threshold_attribute = __ATTR(threshold, 0664, attr_show, attr_store);

static struct attribute *attrs[] = {
    &led_attribute.attr,
    &ldr_attribute.attr,
    &threshold_attribute.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *sys_obj;
static int mock_led __maybe_unused;
static int mock_ldr __maybe_unused = 50;
static int mock_threshold __maybe_unused = 50;

static int clamp_percent(long value)
{
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;
    return value;
}

static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff)
{
    /*
     * attr->attr.name contém o nome do arquivo acessado no sysfs.
     *
     * Exemplos:
     * cat /sys/kernel/smartlamp/led
     * -> attr_name = "led"
     *
     * cat /sys/kernel/smartlamp/ldr
     * -> attr_name = "ldr"
     *
     * cat /sys/kernel/smartlamp/threshold
     * -> attr_name = "threshold"
     */
    const char *attr_name = attr->attr.name;

    /*
     * Variável que armazenará o valor retornado para o usuário.
     */
    int value = 0;

    /*
     * sys_obj não será utilizado nesta implementação.
     * O cast evita warning do compilador.
     */
    (void)sys_obj;

    /*
     * strcmp() compara duas strings.
     *
     * Quando o retorno é 0:
     * significa que as strings são iguais.
     */

    /*
     * Se o arquivo acessado for "led",
     * retorna o valor armazenado em mock_led.
     */
    if (strcmp(attr_name, "led") == 0)
    {
        value = mock_led;
    }

    /*
     * Se o arquivo acessado for "ldr",
     * retorna o valor armazenado em mock_ldr.
     */
    else if (strcmp(attr_name, "ldr") == 0)
    {
        value = mock_ldr;
    }

    /*
     * Se o arquivo acessado for "threshold",
     * retorna o valor armazenado em mock_threshold.
     */
    else if (strcmp(attr_name, "threshold") == 0)
    {
        value = mock_threshold;
    }

    /*
     * sprintf escreve o valor dentro do buffer
     * que será enviado ao usuário.
     *
     * "%d\n"
     * %d  -> inteiro decimal
     * \n  -> quebra de linha
     *
     * Exemplo de saída:
     * 75
     */
    return sprintf(buff, "%d\n", value);
}

static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count)
{
    const char *attr_name = attr->attr.name;
    long value;
    int ret;

    (void)sys_obj;

    ret = kstrtol(buff, 10, &value);
    if (ret)
        return ret;

    value = clamp_percent(value);
    (void)attr_name;

    /*
     * attr_name recebe o nome do arquivo
     * onde o usuário escreveu.
     *
     * Exemplo:
     * echo 75 | sudo tee /sys/kernel/smartlamp/ldr
     *
     * attr_name = "ldr"
     */
    const char *attr_name = attr->attr.name;

    /*
     * value armazenará o número convertido.
     */
    long value;

    /*
     * ret armazenará o retorno da conversão.
     */
    int ret;

    /*
     * sys_obj não será utilizado nesta implementação.
     */
    (void)sys_obj;

    /*
     * kstrtol() converte texto para número inteiro.
     *
     * buff  -> texto recebido do usuário
     * 10    -> base decimal
     * &value -> onde o número convertido será salvo
     *
     * Exemplo:
     * "80" -> 80
     */
    ret = kstrtol(buff, 10, &value);

    /*
     * Se ocorrer erro na conversão,
     * retorna o erro imediatamente.
     */
    if (ret)
        return ret;

    /*
     * clamp_percent() limita o valor entre 0 e 100.
     *
     * Exemplos:
     * -10 -> 0
     * 50  -> 50
     * 150 -> 100
     */
    value = clamp_percent(value);

    /*
     * Verifica qual arquivo foi escrito
     * e atualiza a variável correspondente.
     */

    /*
     * Atualiza mock_led
     */
    if (strcmp(attr_name, "led") == 0)
    {
        mock_led = value;
    }

    /*
     * Atualiza mock_ldr
     */
    else if (strcmp(attr_name, "ldr") == 0)
    {
        mock_ldr = value;
    }

    /*
     * Atualiza mock_threshold
     */
    else if (strcmp(attr_name, "threshold") == 0)
    {
        mock_threshold = value;
    }

    /*
     * Retorna a quantidade de bytes escritos.
     *
     * Isso informa ao kernel que a operação
     * foi concluída com sucesso.
     */
    return count;
}

static int __init smartlamp_mock_init(void)
{
    int ret;

    sys_obj = kobject_create_and_add("smartlamp", kernel_kobj);
    if (!sys_obj)
        return -ENOMEM;

    ret = sysfs_create_group(sys_obj, &attr_group);
    if (ret) {
        kobject_put(sys_obj);
        sys_obj = NULL;
        return ret;
    }

    pr_info("SmartLamp mock: arquivos criados em /sys/kernel/smartlamp\n");
    return 0;
}

static void __exit smartlamp_mock_exit(void)
{
    if (sys_obj) {
        sysfs_remove_group(sys_obj, &attr_group);
        kobject_put(sys_obj);
        sys_obj = NULL;
    }

    pr_info("SmartLamp mock: modulo removido\n");
}

module_init(smartlamp_mock_init);
module_exit(smartlamp_mock_exit);
